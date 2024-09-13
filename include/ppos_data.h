/*
 * PingPongOS - PingPong Operating System
 * Filename: pp_task.h
 * Description: Structure for the TCB (Task Control Block)
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#ifndef PP_TASK_H
#define PP_TASK_H

#include <ucontext.h>

#define STACKSIZE 64 * 1024

#define TASK_READY 0
#define TASK_EXEC 1
#define TASK_FINISH 2

typedef struct task_t {
  struct task_t *prev, *next; // Used in the queue_t
  int tid;                    // id for the task
  int status;                 // ready, executing, finished, ...
  ucontext_t context;         // Current context
  char *stack;                // The stack used by the context
  int initial_priority;       // The start priority of the task (default is 0)
  int current_priority;       // The real priority of the task, can be different
                              // because of aging
} task_t;

#endif // PP_TASK_H
