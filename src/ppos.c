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

static void exec_task_reduce_quantum() {
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

static task_t *scheduler() {
  if (readyQueue->count) {
    task_t *task = readyQueue->taskQueue;
    task_manager_aging(readyQueue);

    // Reset the priority of the task
    task->current_priority = task->initial_priority;
    return task;
  }

  return NULL;
}

static void dispatcher() {
  if (task_manager_remove(readyQueue, dispatcherTask) < 0) {
    log_error("could not be removed from ready queue");
    exit(1);
  }

  if (executingTask->status == TASK_FINISH) {
    log_info("task(%d) finish. execution time: %d ms, processor time: %d ms, "
             "%d activations",
             executingTask->tid, totalSysTime, executingTask->total_time,
             executingTask->num_calls);

    free(executingTask->stack);
    if (executingTask->tid == MAIN_TASK) {
      free(executingTask);
    }
  }

  while (readyQueue->count) {
    executingTask = dispatcherTask;
    task_t *next = scheduler();
    if (next == NULL) {
      log_error("next task(nil)");
      exit(1);
    }

    next->quantum = TASK_QUANTUM;
    if (task_switch(next) < 0) {
      log_error("could not execute task(%d)", next->tid);
      exit(1);
    }

    if (executingTask->status == TASK_FINISH) {
      log_info("task(%d) finish. execution time: %d ms, processor time: %d ms, "
               "%d activations",
               executingTask->tid, totalSysTime, executingTask->total_time,
               executingTask->num_calls);

      free(executingTask->stack);
      if (executingTask->tid == MAIN_TASK) {
        free(executingTask);
      }

    } else if (executingTask->status == TASK_READY) {
      log_debug("inserting executing task(%d) in ready queue",
                executingTask->tid);

      if (task_manager_insert(readyQueue, executingTask) < 0) {
        log_error("failed to insert executing task(%d) in ready queue",
                  executingTask->tid);
        exit(1);
      }
    }

    if (task_manager_remove(readyQueue, dispatcherTask) < 0) {
      log_error("could not be removed from ready queue");
      exit(1);
    }
  }

  log_info("task(%d) finish. execution time: %d ms, processor time: %d ms, "
           "%d activations",
           dispatcherTask->tid, totalSysTime, dispatcherTask->total_time,
           dispatcherTask->num_calls);

  free(dispatcherTask->stack);
  free(dispatcherTask);

  exit(0);
}

static void ppos_init_tasks() {
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

static void ppos_init_timer() {
  action.sa_handler = exec_task_reduce_quantum;
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

  log_set(stderr, 0, LOG_INFO);
  ppos_init_tasks();
  ppos_init_timer();
}

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

  if (threadCount == MAIN_TASK) {
    task->status = TASK_EXEC;
    task->stack = NULL; // Main task does not need to allocate a stack
    getcontext(&(task->context));
  } else {
    task->status = TASK_READY;

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
  task->status = TASK_EXEC;
  temp->status = TASK_READY;

  swapcontext(&(temp->context), &(executingTask->context));
  return 0;
}

void task_exit(int exit_code) {
  log_debug("task(%d)", executingTask->tid);
  executingTask->status = TASK_FINISH;
  swapcontext(&(executingTask->context), &(dispatcherTask->context));
}

int task_id() {
  log_debug("%d", executingTask->tid);
  return executingTask->tid;
}

void task_yield() {
  log_debug("task(%d)", executingTask->tid);
  executingTask->status = TASK_READY;
  dispatcherTask->num_calls++;
  swapcontext(&(executingTask->context), &(dispatcherTask->context));
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

unsigned int systime() { return totalSysTime; }
