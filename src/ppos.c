/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos.c
 * Description: Implementation of the public interface of the OS
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#include "ppos.h"
#include "adt/pptask_manager.h"
#include "debug/log.h"
#include "lib/queue.h"
#include "ppos_data.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ucontext.h>
#include <ucontext.h>

// Task Global structures
static TaskManager *readyQueue = NULL;
static task_t *executingTask = NULL;
static task_t *dispatcherTask = NULL;
static int threadCount = 0;

// Timer Global structures
static struct sigaction action;
static struct itimerval timer;
static unsigned int totalSysTime = 0;

#define TIMER 1000 // 1 ms in microseconds

//=============================================================================
// Scheduler Private Functions
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

  if (executingTask->type != USER) {
    return;
  }

  executingTask->quantum -= 1;
  if (executingTask->quantum <= 0) {
    task_yield();
  }
}

//=============================================================================
// Scheduler Private Functions
//=============================================================================

/**
 * @brief Scheduler function of the OS.
 *
 * This function is responsible into choosing the next task to be executed. And
 * is also responsible into set the priority value of the other tasks in the
 * queue.
 */
static task_t *scheduler() {
  if (readyQueue->count) {
    task_t *task = readyQueue->taskQueue;
    task_manager_aging(readyQueue);

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
 * @brief Wake up all the tasks in the queue.
 *
 * This function is responsible for getting all the tasks that are suspended and
 * put then into the ready queue.
 *
 * @param waiting_queue Pointer for the queue with all the tasks to be awaken
 * @param exit_code exit code of the task that was waited
 */
static void __wakeup(task_t **waiting_queue, int exit_code) {
  task_t *aux = *waiting_queue;
  while (aux) {
    task_awake(aux, waiting_queue);
    aux->waiting_result = exit_code;

    aux->state = TASK_READY;
    if (task_manager_insert(readyQueue, aux) < 0) {
      log_error("failed to insert waiting task(%d) in ready queue", aux->tid);
      exit(1);
    }

    aux = *waiting_queue;
  }
}

/**
 * @brief Wrapper for swapping context with the dispatcher
 */
static void __context_swap_dispatcher() {
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
    if (task_manager_remove(readyQueue, dispatcherTask) < 0) {
      log_error("could not be removed from ready queue");
      exit(1);
    }

    switch (executingTask->state) {
    case TASK_SUSPENDED:
      break;
    case TASK_READY:
      if (task_manager_insert(readyQueue, executingTask) < 0) {
        log_error("failed to insert executing task(%d) in ready queue",
                  executingTask->tid);
        exit(1);
      }
      break;
    case TASK_FINISH:
      __wakeup(&executingTask->waiting_queue, executingTask->exit_result);

      log_info("task(%d) finish. execution time: %d ms, processor time: %d ms, "
               "%d activations",
               executingTask->tid, totalSysTime, executingTask->total_time,
               executingTask->num_calls);

      free(executingTask->stack);
      if (executingTask->tid == MAIN_TASK) {
        free(executingTask);
      }
      break;
    case TASK_EXEC:
      executingTask->state = TASK_READY;
      if (task_manager_insert(readyQueue, executingTask) < 0) {
        log_error("failed to insert executing task(%d) in ready queue",
                  executingTask->tid);
        exit(1);
      }
      break;
    default:
      log_error("invalid state(%d))", executingTask->state);
      exit(1);
    }

    dispatcherTask->state = TASK_EXEC;
    executingTask = dispatcherTask;

    task_t *next = scheduler();
    if (next == NULL) {
      log_debug("next task(nil)");
      continue;
    }

    task_switch(next);
  } while (readyQueue->count);

  log_info("task(%d) finish. execution time: %d ms, processor time: %d ms, "
           "%d activations",
           dispatcherTask->tid, totalSysTime, dispatcherTask->total_time,
           dispatcherTask->num_calls);

  free(dispatcherTask->stack);
  free(dispatcherTask);

  exit(0);
}

//=============================================================================
// General Private Functions
//=============================================================================

/**
 * @brief Initializes the task structures of the OS.
 *
 * Initialize the manager of the queues. Starts the current task
 * (the one that called this function), as the main task, and initialized the
 * dispatcher task.
 */
static void __ppos_init_tasks() {
  readyQueue = task_manager_create();
  if (readyQueue == NULL) {
    log_error("couldn't initiate the ready queue");
    exit(1);
  }

  log_debug("allocating main task");
  executingTask = malloc(sizeof(task_t));
  if (executingTask == NULL) {
    log_error("failed to allocate main task");
    exit(1);
  }

  if (task_init(executingTask, NULL, NULL) < 0) {
    log_error("main task could not be initialized");
    exit(1);
  }

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

/**
 * @brief Initializes a timer interrupt of the OS.
 */
static void __ppos_init_timer() {
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
// General Functions
//=============================================================================

void ppos_init() {
  // Removes the virtual buffer of the output
  // https://en.cppreference.com/w/c/io/setvbuf
  (void)setvbuf(stdout, 0, _IONBF, 0);

  log_set(stderr, 0, LOG_TRACE);
  __ppos_init_tasks();
  __ppos_init_timer();
}

unsigned int systime() { return totalSysTime; }

//=============================================================================
// Task Management
//=============================================================================

int task_init(task_t *task, void (*start_routine)(void *), void *arg) {
  if (task == NULL) {
    log_error("received a task == NULL");
    return -1;
  }

  if (threadCount != MAIN_TASK && start_routine == NULL) {
    log_error("received a start_routine == NULL");
    return -1;
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
      return -1;
    }
    makecontext(&(task->context), (void *)start_routine, 1, arg);

    if (task_manager_insert(readyQueue, task) < 0) {
      log_debug("task(%d) could not be appended in the ready queue", task->tid);
      return -1;
    }
  }

  threadCount++;
  return 0;
}

int task_switch(task_t *task) {
  if (task == NULL) {
    log_debug("received task == NULL");
    return -1;
  }

  log_debug("(%d)->(%d)", executingTask->tid, task->tid);
  task->num_calls++;

  if (task_manager_remove(readyQueue, task) < 0) {
    log_debug("could not remove task(%d) from ready queue", task->tid);
    return -1;
  }

  if (task_manager_insert(readyQueue, executingTask) < 0) {
    log_debug("could not insert task(%d) into ready queue", executingTask->tid);
    return -1;
  }

  task_t *temp = executingTask;
  executingTask = task;
  task->state = TASK_EXEC;
  temp->state = TASK_READY;

  swapcontext(&(temp->context), &(executingTask->context));
  return 0;
}

void task_exit(int exit_code) {
  log_debug("task(%d)", executingTask->tid);
  executingTask->state = TASK_FINISH;
  executingTask->exit_result = exit_code;
  swapcontext(&(executingTask->context), &(dispatcherTask->context));
}

int task_id() {
  log_debug("%d", executingTask->tid);
  return executingTask->tid;
}

void task_yield() {
  log_debug("task(%d)", executingTask->tid);
  executingTask->state = TASK_READY;
  __context_swap_dispatcher();
}

int task_getprio(const task_t *const task) {
  if (task == NULL) {
    return executingTask->initial_priority;
  }

  return task->initial_priority;
}

int task_setprio(task_t *task, int prio) {
  if (prio > TASK_MAX_PRIO || prio < TASK_MIN_PRIO) {
    return -1;
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
      return -1;
    }

    if (task_manager_insert(readyQueue, aux) < 0) {
      log_debug("could not insert task(%d) into ready queue", aux->tid);
      return -1;
    }
  }

  return 0;
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
  executingTask->state = TASK_SUSPENDED;

  log_debug("suspending task(%d)", executingTask->tid);
  if (queue_append((queue_t **)queue, (queue_t *)executingTask) < 0) {
    log_error("could not add task(%d) to the suspend queue",
              executingTask->tid);
    exit(1);
  }

  __context_swap_dispatcher();
}

void task_awake(task_t *task, task_t **queue) {
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
}
