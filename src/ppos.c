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

#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#include <ucontext.h>

static TaskManager *readyQueue = NULL;
static task_t *currentTask = NULL;
static int threadCount = 0;

//=============================================================================
// General Functions
//=============================================================================

void ppos_init() {
  // Removes the virtual buffer of the output
  // https://en.cppreference.com/w/c/io/setvbuf
  setvbuf(stdout, 0, _IONBF, 0);

  log_set(stderr, 1, LOG_TRACE);

  readyQueue = task_manager_create();
  if (readyQueue == NULL) {
    log_error("%s couldn't initiate the ready queue", __func__);
    exit(-1);
  }

  log_debug("%s creating main task", __func__);
  currentTask = malloc(sizeof(task_t));
  if (currentTask == NULL) {
    log_error("%s failed to allocate main task", __func__);
    exit(-1);
  }

  currentTask->next = NULL;
  currentTask->prev = NULL;
  currentTask->tid = threadCount;
  currentTask->status = TASK_EXEC;
  currentTask->stack =
      NULL; // The main task does not need to allocate the stack
  getcontext(&(currentTask->context));
  threadCount++;

  if (task_manager_insert(readyQueue, currentTask) < 0) {
    log_debug("%s could not append task to the queue", __func__);
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

  if (start_routine == NULL) {
    log_error("%s received a start_routine == NULL", __func__);
    return -1;
  }

  task->status = TASK_READY;
  task->tid = threadCount;

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
    log_debug("%s task(%d) could not be appended in the ready queue", __func__,
              task->tid);
    return -1;
  }

  threadCount++;

  return 0;
}

int task_switch(task_t *task) {
  if (task == NULL) {
    log_debug("%s received task == NULL", __func__);
    return -1;
  }

  // Search the task in the queue
  const task_t *aux = readyQueue->taskQueue;
  do {
    if (aux->tid == task->tid) {
      currentTask->status = TASK_READY;
      task->status = TASK_EXEC;

      log_debug("%s swapping task(%d) -> task(%d)", __func__, currentTask->tid,
                task->tid);
      task_t *temp = currentTask;
      currentTask = task;
      swapcontext(&(temp->context), &(currentTask->context));

      return 0;
    }

    aux = aux->next;
  } while (aux != readyQueue->taskQueue);

  log_debug("%s task(%d) not found in the queue", __func__, task->tid);
  return -1;
}

void task_exit(int exit_code) {
  log_debug("%s removing task(%d) from queue", __func__, currentTask->tid);
  if (task_manager_remove(readyQueue, currentTask) < 0) {
    log_error("%s could not remove the task(%d) from queue", __func__,
              currentTask->tid);
    return;
  }

  if (readyQueue->count) {
    currentTask->status = TASK_FINISH;
    currentTask = readyQueue->taskQueue;
    currentTask->status = TASK_EXEC;

    task_manager_print(readyQueue);

    setcontext(&(currentTask->context));
  }
}

int task_id() {
  log_debug("%s %d", __func__, currentTask->tid);
  return currentTask->tid;
}
