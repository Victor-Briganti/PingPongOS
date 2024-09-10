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
#include "lib/queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#include <ucontext.h>

static task_t *taskQueue = NULL;
static task_t *currentTask = NULL;
static int tid = 0;

//=============================================================================
// General Functions
//=============================================================================

void ppos_init() {
  // Removes the virtual buffer of the output
  // https://en.cppreference.com/w/c/io/setvbuf
  setvbuf(stdout, 0, _IONBF, 0);

#ifdef DEBUG
  printf("Creating main task\n");
#endif
  currentTask = malloc(sizeof(task_t));
  if (currentTask == NULL) {
#ifdef DEBUG
    printf("Failed to allocated the main task\n");
#endif
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
#ifdef DEBUG
    printf("Could not append task to the queue\n");
#endif
    free(currentTask);
  }
}

//=============================================================================
// Task Management
//=============================================================================

int task_init(task_t *task, void (*start_routine)(void *), void *arg) {
  if (task == NULL || start_routine == NULL) {
#ifdef DEBUG
    printf("NULL task or routine are not accepted in the initialization of a "
           "task\n");
#endif
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
#ifdef DEBUG
    printf("Stack could not be allocated\n");
#endif
    return -1;
  }

  makecontext(&(task->context), (void *)start_routine, 1, arg);

  if (queue_append((queue_t **)&taskQueue, (queue_t *)task) < 0) {
#ifdef DEBUG
    printf("The task could not be appended in the queue\n");
#endif
    return -1;
  }

  tid++;

  return 0;
}

int task_switch(task_t *task) {
  if (task == NULL) {
#ifdef DEBUG
    printf("Invalid task switch, NULL structure passed\n");
#endif
    return -1;
  }

  // Search the task in the queue
  task_t *aux = taskQueue;
  do {
    if (aux->tid == task->tid) {
#ifdef DEBUG
      printf("Found task on the queue\n");
#endif
      currentTask->status = TASK_READY;
      task->status = TASK_EXEC;

#ifdef DEBUG
      printf("Swappinng context between passed task and current task\n");
#endif
      task_t *temp = currentTask;
      currentTask = task;
      swapcontext(&(temp->context), &(currentTask->context));

      return 0;
    }

    aux = aux->next;
  } while (aux != taskQueue);

#ifdef DEBUG
  printf("Task not found on the queue\n");
#endif
  return -1;
}

void task_exit(int exit_code) {
#ifdef DEBUG
  printf("Removing the current task from the queue\n");
#endif

  if (queue_remove((queue_t **)taskQueue, (queue_t *)currentTask) < 0) {
#ifdef DEBUG
    printf("Could not exit the current task\n");
#endif
    return;
  }

  currentTask->status = TASK_FINISH;
  currentTask = taskQueue;
  currentTask->status = TASK_EXEC;

  setcontext(&(currentTask->context));
}

int task_id() {
#ifdef DEBUG
  printf("Getting current task id\n");
#endif
  return currentTask->tid;
}
