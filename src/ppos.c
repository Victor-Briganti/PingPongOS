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
#include "data/pptask.h"
#include "debug/log.h"
#include "lib/queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#include <ucontext.h>

static task_t *taskQueue = NULL;
static task_t *currentTask = NULL;
static int tid = 0;

/**
 * @brief Closes all the resources opened by the OS
 *
 * Terminates all the tasks that might be opened, and free its structures.
 */
void ppos_terminate() {
  log_debug("Terminating the program\n");
  log_debug("Cleaning Resources\n");

  // This step is similar to a Garbage Collection.
  task_t *aux = taskQueue;
  while (taskQueue) {
    log_debug("Finishing task %d\n", aux->tid);
    task_t *temp = aux;
    aux = aux->next;

    log_debug("Removing\n");
    if (queue_remove((queue_t **)&taskQueue, (queue_t *)temp) < 0) {
      log_debug("Could not remove task from the queue\n");
    }

    if (temp->stack != NULL) {
      log_debug("Freeing stack from task %d\n", temp->tid);
      free(temp->stack);
      temp->stack = NULL;
    }
  }
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

  log_debug("Creating main task\n");
  currentTask = malloc(sizeof(task_t));
  if (currentTask == NULL) {
    log_debug("Failed to allocated the main task\n");
    return;
  }

  currentTask->next = NULL;
  currentTask->prev = NULL;
  currentTask->tid = tid;
  currentTask->status = TASK_EXEC;
  currentTask->stack =
      NULL; // The main task does not need to allocate the stack
  getcontext(&(currentTask->context));
  tid++;

  if (queue_append((queue_t **)&taskQueue, (queue_t *)currentTask) < 0) {
    log_debug("Could not append task to the queue\n");
    free(currentTask);
  }

  atexit(ppos_terminate);
}

//=============================================================================
// Task Management
//=============================================================================

int task_init(task_t *task, void (*start_routine)(void *), void *arg) {
  if (task == NULL || start_routine == NULL) {
    log_debug(
        "NULL task or routine are not accepted in the initialization of a "
        "task\n");
    return -1;
  }

  task->status = TASK_READY;
  task->tid = tid;

  // Initialize the context structure and its stack
  getcontext(&(task->context));
  char *stack = malloc((size_t)STACKSIZE);
  if (stack) {
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = (size_t)STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
  } else {
    log_debug("Stack could not be allocated\n");
    return -1;
  }

  makecontext(&(task->context), (void *)start_routine, 1, arg);

  if (queue_append((queue_t **)&taskQueue, (queue_t *)task) < 0) {
    log_debug("The task could not be appended in the queue\n");
    return -1;
  }

  tid++;

  return 0;
}

int task_switch(task_t *task) {
  if (task == NULL) {
    log_debug("Invalid task switch, NULL structure passed\n");
    return -1;
  }

  // Search the task in the queue
  task_t *aux = taskQueue;
  do {
    if (aux->tid == task->tid) {
      log_debug("Found task on the queue\n");
      currentTask->status = TASK_READY;
      task->status = TASK_EXEC;

      log_debug("Swapping context between passed task and current task\n");
      task_t *temp = currentTask;
      currentTask = task;
      swapcontext(&(temp->context), &(currentTask->context));

      return 0;
    }

    aux = aux->next;
  } while (aux != taskQueue);

  log_debug("Task not found on the queue\n");
  return -1;
}

void task_exit(int exit_code) {
  // Search for finished tasks in queue.
  // This step is similar to a Garbage Collection.
  task_t *aux = taskQueue;
  do {
    if (aux->status == TASK_FINISH) {
      log_debug("Found finished task\n");
      task_t *temp = aux;
      aux = aux->next;

      log_debug("Removing \n");
      if (queue_remove((queue_t **)&taskQueue, (queue_t *)temp) < 0) {
        log_debug("Could not remove finished task from the queue\n");
        break;
      }

      free(temp->stack);
      temp->stack = NULL;
    } else {
      aux = aux->next;
    }

  } while (aux != taskQueue);

  log_debug("Removing the current task from the queue\n");

  if (queue_remove((queue_t **)&taskQueue, (queue_t *)currentTask) < 0) {
    log_debug("Could not exit the current task\n");
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
  log_debug("Getting current task id\n");
  return currentTask->tid;
}
