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
#include "debug/log.h"
#include "lib/queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#include <ucontext.h>

static task_t *taskQueue = NULL;
static task_t *currentTask = NULL;
static int threadCount = 0;

//=============================================================================
// Debugging Functions
//=============================================================================

void qtask_print(void *ptr) {
  task_t *task = (task_t *)ptr;
  if (!task) {
    return;
  }

  printf("%d ", task->tid);
}

//=============================================================================
// General Functions
//=============================================================================

void ppos_init() {
  // Removes the virtual buffer of the output
  // https://en.cppreference.com/w/c/io/setvbuf
  setvbuf(stdout, 0, _IONBF, 0);

#ifdef DEBUG
  log_set(stderr, 1, LOG_TRACE);
#endif

  log_debug("%s creating main task", __func__);
  currentTask = malloc(sizeof(task_t));
  if (currentTask == NULL) {
    log_error("%s failed to allocate main task", __func__);
    return;
  }

  currentTask->next = NULL;
  currentTask->prev = NULL;
  currentTask->tid = threadCount;
  currentTask->status = TASK_EXEC;
  currentTask->stack =
      NULL; // The main task does not need to allocate the stack
  getcontext(&(currentTask->context));
  threadCount++;

  if (queue_append((queue_t **)&taskQueue, (queue_t *)currentTask) < 0) {
    log_debug("%s could not append task to the queue", __func__);
    free(currentTask);
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

  if (queue_append((queue_t **)&taskQueue, (queue_t *)task) < 0) {
    log_debug("%s task(%d) could not be appended in the queue", __func__,
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
  task_t *aux = taskQueue;
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
  } while (aux != taskQueue);

  log_debug("%s task(%d) not found in the queue", __func__, task->tid);
  return -1;
}

void task_exit(int exit_code) {
  // Search for finished tasks in queue.
  // This step is similar to a Garbage Collection.
  task_t *aux = taskQueue;
  do {
    if (aux->status == TASK_FINISH) {
      task_t *temp = aux;
      aux = aux->next;

      log_debug("%s %d", __func__, temp->tid);
      if (queue_remove((queue_t **)&taskQueue, (queue_t *)temp) < 0) {
        log_debug("%s could not remove task(%d) from queue", __func__,
                  temp->tid);
        break;
      }

      free(temp->stack);
      temp->stack = NULL;
    } else {
      aux = aux->next;
    }

  } while (aux != taskQueue);

  log_debug("%s removing task(%d) from queue", __func__, currentTask->tid);
  if (queue_remove((queue_t **)&taskQueue, (queue_t *)currentTask) < 0) {
    log_error("%s could not remove the task(%d) from queue", __func__,
              currentTask->tid);
    return;
  }

  if (taskQueue != NULL) {
    currentTask->status = TASK_FINISH;
    currentTask = taskQueue;
    currentTask->status = TASK_EXEC;

    setcontext(&(currentTask->context));
  }
}

int task_id() {
  log_debug("%s %d", __func__, currentTask->tid);
  return currentTask->tid;
}
