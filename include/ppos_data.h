/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos_data.h
 * Description: Common data structure used by the OS
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#ifndef PP_DATA_H
#define PP_DATA_H

#include <ucontext.h>

#define STACKSIZE 64 * 1024

#define TASK_READY 0
#define TASK_EXEC 1
#define TASK_FINISH 2

#define TASK_MAX_PRIO 20
#define TASK_MIN_PRIO -20

#define TASK_QUANTUM 20 // In milliseconds

// Reserved IDs for special Tasks
#define MAIN_TASK 0
#define DISPATCHER_TASK 1

typedef enum task_type {
  USER,
  SYSTEM,
} task_type;

// Structure for the TCB (Task Control Block)
typedef struct task_t {
  struct task_t *prev, *next; // Used in the queue_t
  int tid;                    // id for the task
  int status;                 // ready, executing, finished, ...
  ucontext_t context;         // Current context
  char *stack;                // The stack used by the context
  int initial_priority;       // The start priority of the task (default is 0)
  int current_priority;       // The real priority of the task, can be different
                              // because of aging
  task_type type;             // Defines the type of the task executing
  unsigned int quantum;       // Total quantum that the task has to execute
  unsigned int total_time;    // Total time of execution on CPU
  unsigned int current_time;  // Current system time of the task execution
  unsigned int num_calls;     // Number of times the task was dispatched
} task_t;

#endif // PP_DATA_H
