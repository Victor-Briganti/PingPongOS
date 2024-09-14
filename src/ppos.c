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

#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#include <ucontext.h>

// Reserved IDs for special Tasks
#define MAIN_TASK 0
#define DISPATCHER_TASK 1

static TaskManager *readyQueue = NULL;
static task_t *executingTask = NULL;
static task_t *dispatcherTask = NULL;
static int threadCount = 0;

//=============================================================================
// Scheduler Private Functions
//=============================================================================

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
    exit(-1);
  }

  if (executingTask->status == TASK_FINISH) {
    log_info("executing task(%d) TASK_FINISH", executingTask->tid);
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
      exit(-1);
    }

    if (task_switch(next) < 0) {
      log_error("could not execute task(%d)", next->tid);
      exit(-1);
    }

    if (executingTask->status == TASK_FINISH) {
      log_info("executing task(%d) TASK_FINISH", executingTask->tid);
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
        exit(-1);
      }
    }

    if (task_manager_remove(readyQueue, dispatcherTask) < 0) {
      log_error("could not be removed from ready queue");
      exit(-1);
    }
  }

  exit(0);
}

//=============================================================================
// General Functions
//=============================================================================

void ppos_init() {
  // Removes the virtual buffer of the output
  // https://en.cppreference.com/w/c/io/setvbuf
  (void)setvbuf(stdout, 0, _IONBF, 0);

  log_set(stderr, 0, LOG_TRACE);

  readyQueue = task_manager_create();
  if (readyQueue == NULL) {
    log_error("couldn't initiate the ready queue");
    exit(-1);
  }

  log_debug("allocating main task");
  executingTask = malloc(sizeof(task_t));
  if (executingTask == NULL) {
    log_error("failed to allocate main task");
    exit(-1);
  }

  if (task_init(executingTask, NULL, NULL) < 0) {
    log_error("main task could not be initialized");
    exit(-1);
  }

  dispatcherTask = malloc(sizeof(task_t));
  if (dispatcherTask == NULL) {
    log_error("failed to allocate dispatcher task");
    exit(-1);
  }

  if (task_init(dispatcherTask, dispatcher, NULL) < 0) {
    log_error("dispatcher task could not be initialized");
    exit(-1);
  }
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
