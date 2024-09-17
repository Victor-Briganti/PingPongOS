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

#define STACKSIZE (64 * 1024)

#define TASK_MAX_PRIO (20)
#define TASK_MIN_PRIO (-20)

#define TASK_QUANTUM (20) // In milliseconds

// Reserved IDs for special Tasks
#define MAIN_TASK (0)
#define DISPATCHER_TASK (1)

typedef enum task_state {
  TASK_READY,
  TASK_EXEC,
  TASK_FINISH,
  TASK_SUSPENDED,
} task_state;

typedef enum task_type {
  USER,
  SYSTEM,
} task_type;

// Structure for the TCB (Task Control Block)
typedef struct task_t {
  // Used in the queue_t
  struct task_t *prev, *next;

  // id for the task
  int tid;

  // ready, executing, finished, ...
  task_state state;

  // Current context
  ucontext_t context;

  // The stack used by the context
  char *stack;

  // The start priority of the task (default is 0)
  int initial_priority;

  // The real priority of the task.
  int current_priority;

  // Defines the type of the task executing
  task_type type;

  // Total quantum that the task has to execute
  unsigned int quantum;

  // Total time of execution on CPU
  unsigned int total_time;

  // Current system time of the task execution
  unsigned int current_time;

  // Mark the time that the task is going to sleep
  unsigned int sleep_time;

  // Number of times the task was dispatched
  unsigned int num_calls;

  // The exit result of this task
  int exit_result;

  // Queue of tasks waiting this one to finish executing
  struct task_t *waiting_queue;

  // Return value of the task waited
  int waiting_result;

} task_t;

#endif // PP_DATA_H
