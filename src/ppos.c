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
    return readyQueue->taskQueue;
  }

  return NULL;
}

static void dispatcher() {
  if (task_manager_remove(readyQueue, dispatcherTask) < 0) {
    log_error("%s could not be removed from ready queue", __func__);
    exit(-1);
  }

  if (executingTask->status == TASK_FINISH) {
    log_info("%s executing task(%d) TASK_FINISH", __func__, executingTask->tid);
    free(executingTask->stack);
    if (executingTask->tid == MAIN_TASK) {
      free(executingTask);
    }
  }

  while (readyQueue->count) {
    executingTask = dispatcherTask;
    task_t *next = scheduler();
    if (next == NULL) {
      log_error("%s next task(nil)");
      exit(-1);
    }

    if (task_switch(next) < 0) {
      log_error("%s could not execute task(%d)", __func__, next->tid);
      exit(-1);
    }

    if (executingTask->status == TASK_FINISH) {
      log_info("%s executing task(%d) TASK_FINISH", __func__,
               executingTask->tid);
      free(executingTask->stack);
      if (executingTask->tid == MAIN_TASK) {
        free(executingTask);
      }
    }

    if (task_manager_remove(readyQueue, dispatcherTask) < 0) {
      log_error("%s could not be removed from ready queue", __func__);
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

  log_set(stderr, 1, LOG_TRACE);

  readyQueue = task_manager_create();
  if (readyQueue == NULL) {
    log_error("%s couldn't initiate the ready queue", __func__);
    exit(-1);
  }

  log_debug("%s allocating main task", __func__);
  executingTask = malloc(sizeof(task_t));
  if (executingTask == NULL) {
    log_error("%s failed to allocate main task", __func__);
    exit(-1);
  }

  if (task_init(executingTask, NULL, NULL) < 0) {
    log_error("%s main task could not be initialized", __func__);
    exit(-1);
  }

  dispatcherTask = malloc(sizeof(task_t));
  if (dispatcherTask == NULL) {
    log_error("%s failed to allocate dispatcher task", __func__);
    exit(-1);
  }

  if (task_init(dispatcherTask, dispatcher, NULL) < 0) {
    log_error("%s dispatcher task could not be initialized", __func__);
    exit(-1);
  }
}

//=============================================================================
// Task Management
//=============================================================================

int task_init(task_t *task, void (*start_routine)(void *), void *arg) {
  if (task == NULL) {
    log_error("%s received a task == NULL", __func__);
    return -1;
  }

  if (threadCount != MAIN_TASK && start_routine == NULL) {
    log_error("%s received a start_routine == NULL", __func__);
    return -1;
  }

  task->next = NULL;
  task->prev = NULL;
  task->tid = threadCount;

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
      log_error("%s stack could not be allocated", __func__);
      return -1;
    }
    makecontext(&(task->context), (void *)start_routine, 1, arg);

    if (task_manager_insert(readyQueue, task) < 0) {
      log_debug("%s task(%d) could not be appended in the ready queue",
                __func__, task->tid);
      return -1;
    }
  }

  threadCount++;
  return 0;
}

int task_switch(task_t *task) {
  if (task == NULL) {
    log_debug("%s received task == NULL", __func__);
    return -1;
  }

  log_debug("%s (%d)->(%d)", __func__, executingTask->tid, task->tid);

  task_manager_print(readyQueue);
  if (task_manager_remove(readyQueue, task) < 0) {
    log_debug("%s could not remove task(%d) from ready queue", __func__,
              task->tid);
    return -1;
  }
  task_manager_print(readyQueue);

  if (task_manager_insert(readyQueue, executingTask) < 0) {
    log_debug("%s could not insert task(%d) into ready queue", __func__,
              executingTask->tid);
    return -1;
  }
  task_manager_print(readyQueue);

  task_t *temp = executingTask;
  executingTask = task;
  task->status = TASK_EXEC;
  temp->status = TASK_READY;

  swapcontext(&(temp->context), &(executingTask->context));
  return 0;
}

void task_exit(int exit_code) {
  log_debug("%s task(%d)", __func__, executingTask->tid);
  executingTask->status = TASK_FINISH;
  swapcontext(&(executingTask->context), &(dispatcherTask->context));
}

int task_id() {
  log_debug("%s %d", __func__, executingTask->tid);
  return executingTask->tid;
}
