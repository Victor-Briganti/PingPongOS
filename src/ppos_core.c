/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos.c
 * Description: Implementation of the public interface of the OS
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#include "adt/pptask_manager.h"
#include "debug/log.h"
#include "lib/queue.h"
#include "ppos.h"
#include "ppos_data.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ucontext.h>
#include <ucontext.h>

// Task Global structures
static TaskManager *readyQueue = NULL;
static TaskManager *sleepQueue = NULL;
static task_t *executingTask = NULL;
static task_t *dispatcherTask = NULL;
static int numSuspedingTasks = 0;
static int globalLock = 0;

// Timer Global structur
static unsigned int totalSysTime = 0;

#define TIMER 1000 // 1 ms in microseconds

//=============================================================================
// Timer Private Functions
//=============================================================================

/**
 * @brief Timer interrupt function of the OS
 *
 * The main function of this task is to keep up with the total time of execution
 * of the system, and to manage the total quantum that the current executing
 * task already has consumed of execution, if the executing task has already
 * consumed all its quantum yield it.
 */
static void __time_tick() {
  totalSysTime++;

  if (executingTask->current_time) {
    executingTask->total_time += totalSysTime - executingTask->current_time;
  }
  executingTask->current_time = totalSysTime;

  if (executingTask->type == SYSTEM || globalLock) {
    return;
  }

  executingTask->quantum -= 1;
  if (executingTask->quantum <= 0 || sleepQueue->count) {
    task_yield();
  }
}

/**
 * @brief Initializes a timer interrupt of the OS.
 */
static void __ppos_init_timer() {
  static struct sigaction action;
  static struct itimerval timer;

  action.sa_handler = __time_tick;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction(SIGALRM, &action, 0) < 0) {
    log_error("erro no sigaction");
    exit(1);
  }

  timer.it_value.tv_usec = TIMER;
  timer.it_value.tv_sec = 0;
  timer.it_interval.tv_usec = TIMER;
  timer.it_interval.tv_sec = 0;

  if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
    log_error("erro no setitimer");
    exit(1);
  }
}

//=============================================================================
// Scheduler Private Functions
//=============================================================================

#define PRIORITY_AGING (-1)

/**
 * @brief Function to age the priorities of the queue
 *
 * This function is used with the map function of the queue. And is used to age
 * the priority of everyone in the queue.
 */
static void __aging(void *ptr) {
  task_t *task = (task_t *)ptr;

  if (task == NULL) {
    return;
  }

  if (task->current_priority > TASK_MIN_PRIO) {
    task->current_priority += PRIORITY_AGING;
  }
}

/**
 * @brief Scheduler function of the OS.
 *
 * This function is responsible into choosing the next task to be executed. And
 * is also responsible into set the priority value of the other tasks in the
 * queue.
 */
static task_t *scheduler() {
  if (readyQueue->taskQueue) {
    task_t *task = readyQueue->taskQueue;
    task_manager_map(readyQueue, __aging);

    // Reset the priority of the task
    task->current_priority = task->initial_priority;

    // Reset the quantum of the task
    task->quantum = TASK_QUANTUM;
    return task;
  }

  return NULL;
}

//=============================================================================
// Dispatcher Private Functions
//=============================================================================

/**
 * @brief Wake up all the tasks awaiting in the queue.
 *
 * This function is responsible for getting all the tasks that are suspended and
 * put then into the ready queue.
 *
 * @param waiting_queue Pointer for the queue with all the tasks to be awaken
 * @param exit_code exit code of the task that was waited
 */
static void __wakeup_await(task_t **waiting_queue, int exit_code) {
  task_t *aux = *waiting_queue;
  while (aux) {
    task_awake(aux, waiting_queue);
    aux->waiting_result = exit_code;
    aux = *waiting_queue;
  }
}

/**
 * @brief Wake up all the tasks that passed the sleeping time.
 *
 * This function is responsible for getting all the tasks that should not be
 * sleeping anymore and put then into the ready queue.
 *
 * @param waiting_queue Pointer for the queue with all the tasks to be awaken
 * @param exit_code exit code of the task that was waited
 */
static void __wakeup_sleep(task_t **waiting_queue) {
  task_t *aux = *waiting_queue;
  do {
    if (aux && aux->sleep_time <= totalSysTime) {
      if (task_manager_remove(sleepQueue, aux) < 0) {
        log_error("failed to remove sleep task(%d) of sleep queue", aux->tid);
        exit(1);
      }

      aux->state = TASK_READY;
      aux->sleep_time = 0;
      if (task_manager_insert(readyQueue, aux) < 0) {
        log_error("failed to insert waiting task(%d) in ready queue", aux->tid);
        exit(1);
      }

      numSuspedingTasks--;
      aux = *waiting_queue;
    } else {
      break;
    }
  } while (aux);
}

/**
 * @brief Wrapper for swapping context with the dispatcher
 *
 * @param state The new state for the executing task. This should not be a
 * TASK_EXEC.
 */
static void __context_swap_dispatcher(task_state state) {
  if (state == TASK_EXEC) {
    log_error("invalid task state");
    exit(1);
  }

  executingTask->state = state;
  dispatcherTask->num_calls++;
  swapcontext(&(executingTask->context), &(dispatcherTask->context));
}

/**
 * @brief Dispatcher task of the OS.
 *
 * This function is responsible to add or remove tasks into the queues and to
 * execute the task that the scheduler has choose.
 */
static void dispatcher() {
  do {

    task_t *currentTask = executingTask;
    dispatcherTask->state = TASK_EXEC;
    executingTask = dispatcherTask;

    if (task_manager_search(readyQueue, dispatcherTask) == 0) {
      if (task_manager_remove(readyQueue, dispatcherTask) < 0) {
        log_error("could not be removed from ready queue");
        exit(1);
      }
    }

    switch (currentTask->state) {
    case TASK_EXEC:      // Only the dispatcher can be in here
    case TASK_SUSPENDED: // Is already in another queue
      break;
    case TASK_READY:
      if (task_manager_insert(readyQueue, currentTask) < 0) {
        log_error("failed to insert executing task(%d) in ready queue",
                  currentTask->tid);
        exit(1);
      }
      break;
    case TASK_FINISH:
      __wakeup_await(&currentTask->waiting_queue, currentTask->exit_result);

      log_info("task(%d) finish. execution time: %d ms, processor time: %d ms, "
               "%d activations",
               currentTask->tid, totalSysTime, currentTask->total_time,
               currentTask->num_calls);

      free(currentTask->stack);
      if (currentTask->tid == MAIN_TASK) {
        free(currentTask);
      }
      break;
    default:
      log_error("invalid state(%d))", currentTask->state);
      exit(1);
    }

    __wakeup_sleep(&(sleepQueue->taskQueue));

    task_t *next = scheduler();
    if (next == NULL) {
      log_debug("next task(nil)");
      continue;
    }

    task_switch(next);
  } while (readyQueue->taskQueue || sleepQueue->taskQueue || numSuspedingTasks);

  log_info("task(%d) finish. execution time: %d ms, processor time: %d ms, "
           "%d activations",
           dispatcherTask->tid, totalSysTime, dispatcherTask->total_time,
           dispatcherTask->num_calls);

  free(dispatcherTask->stack);
  free(dispatcherTask);

  exit(0);
}

//=============================================================================
// Queue Managers Private Functions
//=============================================================================

/**
 * @brief Compare the priority of two tasks
 *
 * @param ptr1 Pointer for the element that is going to be compared
 * @param ptr2 Pointer for the element in the queue
 *
 * @return 0 if equal, 0< if elem has a higher priority, 0> if elem has a lower
 * priority.
 */
static int __task_comp_prio(const void *ptr1, const void *ptr2) {
  assert(ptr1 != NULL);
  assert(ptr2 != NULL);
  task_t *elem = (task_t *)ptr1;
  task_t *queue = (task_t *)ptr2;

  return elem->initial_priority - queue->current_priority;
}

/**
 * @brief Initializer for the ready queue.
 */
static void __ppos_init_ready_queue() {
  readyQueue = task_manager_create("ready", __task_comp_prio);
  if (readyQueue == NULL) {
    log_error("couldn't initiate queue");
    exit(1);
  }
}

/**
 * @brief Compare the sleep time of two tasks
 *
 * @param ptr1 Pointer for the element that is going to be compared
 * @param ptr2 Pointer for the element in the queue
 *
 * @return 0 if equal, 0< if elem has a higher sleep time, 0> if elem has a
 * lower sleep time.
 */
static int __task_comp_time(const void *ptr1, const void *ptr2) {
  assert(ptr1 != NULL);
  assert(ptr2 != NULL);
  task_t *elem = (task_t *)ptr1;
  task_t *queue = (task_t *)ptr2;

  if (elem->sleep_time == queue->sleep_time) {
    return 0;
  } else if (elem->sleep_time > queue->sleep_time) {
    return 1;
  } else {
    return -1;
  }
}

/**
 * @brief Initializer for the sleep queue.
 */
static void __ppos_init_sleep_queue() {
  sleepQueue = task_manager_create("sleep", __task_comp_time);
  if (sleepQueue == NULL) {
    log_error("couldn't initiate queue");
    exit(1);
  }
}

/**
 * @brief Initializes the task structures of the OS.
 *
 * Initialize the manager of the queues. Starts the current task
 * (the one that called this function), as the main task, and initialized the
 * dispatcher task.
 */
static void __ppos_init_main_task() {
  executingTask = malloc(sizeof(task_t));
  if (executingTask == NULL) {
    log_error("failed to allocate");
    exit(1);
  }

  if (task_init(executingTask, NULL, NULL) < 0) {
    log_error("could not be initialized");
    exit(1);
  }
}

/**
 * @brief Initializes the task structures of the OS.
 *
 * Initialize the manager of the queues. Starts the current task
 * (the one that called this function), as the main task, and initialized the
 * dispatcher task.
 */
static void __ppos_init_disp_task() {
  dispatcherTask = malloc(sizeof(task_t));
  if (dispatcherTask == NULL) {
    log_error("failed to allocate dispatcher task");
    exit(1);
  }

  if (task_init(dispatcherTask, dispatcher, NULL) < 0) {
    log_error("dispatcher task could not be initialized");
    exit(1);
  }
  dispatcherTask->type = SYSTEM;
}

//=============================================================================
// General Public Functions
//=============================================================================

void ppos_init() {
  // Removes the virtual buffer of the output
  // https://en.cppreference.com/w/c/io/setvbuf
  (void)setvbuf(stdout, 0, _IONBF, 0);

  log_set(stderr, 0, LOG_FATAL);

  __ppos_init_ready_queue();
  __ppos_init_sleep_queue();
  __ppos_init_main_task();
  __ppos_init_disp_task();
  __ppos_init_timer();
}

unsigned int systime() { return totalSysTime; }

//=============================================================================
// Task Public Management
//=============================================================================

int task_init(task_t *task, void (*start_routine)(void *), void *arg) {
  globalLock = 1;
  static int threadCount = 0;

  if (task == NULL) {
    log_error("received a task == NULL");
    goto error;
  }

  if (threadCount != MAIN_TASK && start_routine == NULL) {
    log_error("received a start_routine == NULL");
    goto error;
  }

  task->next = NULL;
  task->prev = NULL;
  task->tid = threadCount;
  task->initial_priority = 0;
  task->current_priority = 0;
  task->type = USER;
  task->quantum = TASK_QUANTUM;
  task->total_time = 0;
  task->current_time = 0;
  task->sleep_time = 0;
  task->num_calls = 0;
  task->exit_result = 0;
  task->waiting_queue = NULL;
  task->waiting_result = 0;

  if (threadCount == MAIN_TASK) {
    task->state = TASK_EXEC;
    task->stack = NULL; // Main task does not need to allocate a stack
    getcontext(&(task->context));
  } else {
    task->state = TASK_READY;

    // Initialize the context structure and its stack
    getcontext(&(task->context));
    char *stack = malloc((size_t)STACKSIZE);
    if (stack) {
      task->context.uc_stack.ss_sp = stack;
      task->context.uc_stack.ss_size = (size_t)STACKSIZE;
      task->context.uc_stack.ss_flags = 0;
      task->context.uc_link = 0;
    } else {
      log_error("stack could not be allocated");
      goto error;
    }
    makecontext(&(task->context), (void *)start_routine, 1, arg);

    if (task_manager_insert(readyQueue, task) < 0) {
      log_debug("task(%d) could not be appended in the ready queue", task->tid);
      goto error;
    }
  }

  threadCount++;
  globalLock = 0;
  return 0;
error:
  globalLock = 0;
  return -1;
}

int task_switch(task_t *task) {
  globalLock = 1;
  if (task == NULL) {
    log_debug("received task == NULL");
    goto error;
  }

  log_debug("(%d)->(%d)", executingTask->tid, task->tid);
  task->num_calls++;

  if (task_manager_remove(readyQueue, task) < 0) {
    log_debug("could not remove task(%d) from ready queue", task->tid);
    goto error;
  }

  if (task_manager_insert(readyQueue, executingTask) < 0) {
    log_debug("could not insert task(%d) into ready queue", executingTask->tid);
    goto error;
  }

  task_t *temp = executingTask;
  executingTask = task;
  task->state = TASK_EXEC;
  temp->state = TASK_READY;

  swapcontext(&(temp->context), &(executingTask->context));
  return 0;
error:
  globalLock = 1;
  return -1;
}

void task_exit(int exit_code) {
  log_debug("task(%d)", executingTask->tid);
  executingTask->exit_result = exit_code;
  __context_swap_dispatcher(TASK_FINISH);
}

int task_id() {
  log_debug("%d", executingTask->tid);
  return executingTask->tid;
}

void task_yield() {
  log_debug("task(%d)", executingTask->tid);
  __context_swap_dispatcher(TASK_READY);
}

int task_getprio(const task_t *const task) {
  if (task == NULL) {
    return executingTask->initial_priority;
  }

  return task->initial_priority;
}

int task_setprio(task_t *task, int prio) {
  globalLock = 1;
  if (prio > TASK_MAX_PRIO || prio < TASK_MIN_PRIO) {
    goto error;
  }

  task_t *aux = NULL;
  if (task == NULL) {
    aux = executingTask;
  } else {
    aux = task;
  }

  int diffPriority = aux->initial_priority - aux->current_priority;
  aux->current_priority = prio - diffPriority;
  aux->initial_priority = prio;

  // Reinsert the task into the queue with its new priority
  if (aux != executingTask) {
    if (task_manager_remove(readyQueue, aux) < 0) {
      log_debug("could not remove task(%d) from ready queue", aux->tid);
      goto error;
    }

    if (task_manager_insert(readyQueue, aux) < 0) {
      log_debug("could not insert task(%d) into ready queue", aux->tid);
      goto error;
    }
  }

  return 0;
error:
  globalLock = 0;
  return -1;
}

int task_wait(task_t *task) {
  if (task == NULL) {
    log_error("receive a NULL task");
    return -1;
  }

  if (task->state == TASK_FINISH) {
    log_error("task(%d) already finished", task->tid);
    return -1;
  }

  log_debug("task(%d) waiting task(%d)", executingTask->tid, task->tid);
  task_suspend(&(task->waiting_queue));
  return executingTask->waiting_result;
}

void task_suspend(task_t **queue) {
  globalLock = 1;
  log_debug("suspending task(%d)", executingTask->tid);

  if (queue_append((queue_t **)queue, (queue_t *)executingTask) < 0) {
    log_error("could not add task(%d) to the suspend queue",
              executingTask->tid);
    exit(1);
  }

  numSuspedingTasks++;
  globalLock = 1;
  __context_swap_dispatcher(TASK_SUSPENDED);
}

void task_awake(task_t *task, task_t **queue) {
  globalLock = 1;
  if (task == NULL) {
    log_error("received a NULL task");
    exit(1);
  }

  if (queue == NULL) {
    log_error("received a NULL queue");
    exit(1);
  }

  if (queue_remove((queue_t **)queue, (queue_t *)task) < 0) {
    log_error("could not awake task(%d)", task->tid);
    exit(1);
  }

  task->state = TASK_READY;
  if (task_manager_insert(readyQueue, task) < 0) {
    log_error("failed to insert waiting task(%d) in ready queue", task->tid);
    exit(1);
  }

  numSuspedingTasks--;
  globalLock = 0;
}

void task_sleep(int time) {
  globalLock = 1;

  log_debug("sleeping task(%d)", executingTask->tid);
  if (time < 0) {
    log_debug("time passed is negative");
    return;
  }

  executingTask->sleep_time = (unsigned int)time + totalSysTime;

  if (task_manager_insert(sleepQueue, executingTask) < 0) {
    log_error("could not add task(%d) to the suspend queue",
              executingTask->tid);
    exit(1);
  }

  numSuspedingTasks++;
  globalLock = 0;
  __context_swap_dispatcher(TASK_SUSPENDED);
}
