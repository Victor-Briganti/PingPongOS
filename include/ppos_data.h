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

//=============================================================================
// Task Structure
//=============================================================================

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

//=============================================================================
// Mutex Structure
//=============================================================================

// Structure for the Mutex
typedef struct mutex_t {
  // Mutual exclusion
  int lock;
} mutex_t;

//=============================================================================
// Semaphore Structure
//=============================================================================

typedef enum semaphore_state {
  SEM_CREATED,
  SEM_INITALIZED,
  SEM_FINISHED,
} semaphore_state;

// Structure for the Semaphore
typedef struct semaphore_t {
  // Mutual exclusion
  int lock;

  // Flag to verify the state of the semaphore
  semaphore_state state;

  // Queue of waiting tasks
  task_t *queue;
} semaphore_t;

//=============================================================================
// Barrier Structure
//=============================================================================

typedef enum barrier_state {
  BAR_INITALIZED,
  BAR_FINISHED,
} barrier_state;

// Structure for the Semaphore
typedef struct barrier_t {
  // Number of tasks that needs to wait
  int num_tasks;

  // Flag to verify the state of the barrier
  barrier_state state;

  // Queue of waiting tasks
  task_t *queue;
} barrier_t;

//=============================================================================
// Message Queue Structure
//=============================================================================

typedef enum mqueue_state {
  MQE_INITALIZED,
  MQE_FINISHED,
} mqueue_state;

// Structure for the Message Queue
typedef struct mqueue_t {
  // Array of messages
  void *msgs;

  // Current index of the array
  int index;

  // Max number of messagens
  int max_msgs;

  // Number of current messagens
  int num_msgs;

  // Flag to verify the state of the message queue
  mqueue_state state;

  // The size of the stored elements
  size_t msg_size;

  // Semaphore for producer
  semaphore_t sem_prod;

  // Semaphore for consumer
  semaphore_t sem_cons;
} mqueue_t;

#endif // PP_DATA_H
