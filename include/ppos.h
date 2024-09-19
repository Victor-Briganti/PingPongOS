/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos.h
 * Description: Header for the public interface of the OS
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#ifndef PPOS_H
#define PPOS_H

#include "ppos_data.h"

// Enables the POSIX compatibility on MacOS X
#define _XOPEN_SOURCE 600

// Otimizations can get in the way of code that manipulate context
#ifdef __OPTIMIZE__
#error "Please do not use optimization (-O1, -O2, ...)"
#endif

//=============================================================================
// General Functions
//=============================================================================

/**
 * @brief Initialize the OS
 *
 * This function must be called in the main().
 */
void ppos_init();

/**
 * @brief Gets the total execution time of the system.
 *
 * @return The total time of the system since execution.
 */
unsigned int systime();

//=============================================================================
// Task Management
//=============================================================================

/**
 * @brief Initializes a new task.
 *
 * Initializes with this respective routing and arguments. This does not change
 * the current execution context for that you need to call task_switch.
 *
 * @param task Pointer that describes the task
 * @param start_func Function pointer that the task is going to execute
 * @param arg Arguments that are going to be used by the start_func
 *
 * @return The id of the task (0>) or a error.
 */
int task_init(task_t *task, void (*start_routine)(void *), void *arg);

/**
 * @brief Switches to other task
 *
 * Passes the control of the processor to another task
 *
 * @param task The task that is going to assume the control
 * @return 0 if the switch was successful, and 0< otherwise.
 */
int task_switch(task_t *task);

/**
 * @brief Ends the current task.
 *
 * @param exit_code Termination code returned by the task
 */
void task_exit(int exit_code);

/**
 * @brief Gets the current task id
 *
 * This identifies the current task. If is the main task the value
 * returned is 0.
 *
 * @return The id of the task. 0 if is the main.
 */
int task_id();

/**
 * @brief Yields the current task.
 *
 * Passes control of the CPU back to the scheduler, allowing it to select and
 * execute another task or thread.
 */
void task_yield();

/**
 * @brief Gets the priority of a task.
 *
 * @param task Pointer for the task that we want to return the priority. If NULL
 * is passed the priority of the executing task is returned.
 *
 * @return The priority of the task.
 */
int task_getprio(const task_t *const task);

/**
 * @brief Changes the priority of the task.
 *
 * @param task Pointer for the task that we want to return the priority. If NULL
 * the priority of the executing task is set.
 * @param prio The new priority that is going to be set. The value needs to be
 * between -20 and +20.
 *
 * @return 0 if the priority could be adjusted, or 0< otherwise.
 */
int task_setprio(task_t *task, int prio);

/**
 * @brief Waits for a task to complete.
 *
 * Suspend the current executing task until the task passed as an argument has
 * executed. After that the task is put in the ready queue again.
 * More than one task can wait for the task being passed as an argument.
 *
 * @param task The pointer for the task being waited.
 *
 * @return The exit value of the task being waited.
 */
int task_wait(task_t *task);

/**
 * @brief Suspends the current task.
 *
 * Place the current task into the suspending queue.
 *
 * @param queue Suspending queue that is going to receive the suspended task.
 */
void task_suspend(task_t **queue);

/**
 * @brief Awake the suspended task.
 *
 * Removes the task passed of the queue. And insert it into the ready queue.
 *
 * @param task Pointer for the task that needs to be awake.
 * @param queue Pointer for the queue of suspended tasks.
 */
void task_awake(task_t *task, task_t **queue);

/**
 * @brief Make the current task sleep.
 *
 * Yields the current task and place it in the sleep queue. Where its going to
 * sleep the specified time.
 *
 * @param task Pointer for the task that needs to be awake.
 * @param queue Pointer for the queue of suspended tasks.
 */
void task_sleep(int time);

//=============================================================================
// Mutex Management
//=============================================================================

/**
 * @brief Initializes the mutex structure.
 *
 * Initializes the mutex in a unlocked state.
 *
 * @param mutex Pointer for the mutex that is going to be initialized
 *
 * @return 0 if successfuly initialized, and -1 if something went wrong.
 */
int mutex_init(mutex_t *mutex);

/**
 * @brief Destroy the mutex structure
 *
 * Destroy the mutex passed by the pointer.
 *
 * @param mutex Pointer for the mutex to be destroyed
 *
 * @return 0 if successfuly destroyed, and -1 otherwise.
 */
int mutex_destroy(mutex_t *mutex);

/**
 * @brief Lock this mutex
 *
 * Lock this mutex preventing another task from entering this location. Calls in
 * the mutex are non blocking.
 *
 * @param mutex Pointer for the mutex that is going to be locked.
 *
 * @return 1 if the mutex was locked, and 0 otherwise, and -1 if something went
 * wrong.
 */
int mutex_lock(mutex_t *mutex);

/**
 * @brief Unlock this mutex
 *
 * Unlock the mutex passed. Calls in the lock and unlock are non blocking.
 *
 * @param task Pointer for the mutex that is going to be unlocked
 *
 * @return 0 if the mutex could be unlocked, and 1 otherwise.
 */
int mutex_unlock(mutex_t *mutex);

//=============================================================================
// Semaphore Management
//=============================================================================

/**
 * @brief Initializes a new semaphore.
 *
 * Initializes the semaphore structure with a initial value and a empty queue.
 *
 * @param sem Pointer for the semaphore
 * @param value The start value of the semaphore. If the value is negative, this
 * semaphore starts locked.
 *
 * @return 0 if successfuly created, and -1 if something went wrong.
 */
int sem_init(semaphore_t *sem, int value);

/**
 * @brief Destroy the semaphore
 *
 * Destroy the semaphore passed by the pointer, and wake up all the tasks that
 * were waiting for this semaphore.
 *
 * @param sem Pointer for the semaphore to be destroyed
 *
 * @return 0 if successfuly destroyed, and -1 otherwise.
 */
int sem_destroy(semaphore_t *sem);

/**
 * @brief Releases the semaphore
 *
 * Release the semaphore, adding 1 to the value stored inside the structure.
 * This call is non blocking. If there is some task waiting in the queue, the
 * first one in the queue is going to be placed in the ready queue.
 *
 * @param sem Pointer for the semaphore that is going to be released
 *
 * @return 0 if  the switch was successful, and 0< otherwise.
 */
int sem_up(semaphore_t *sem);

/**
 * @brief Locks this semaphore
 *
 * Try to lock the semaphore passed. This call can block, if the value inside
 * the semaphore is negative, the current task is suspended, and inserted in the
 * end of the queue of the semaphore.
 *
 * @param task Pointer for the semaphore that is going to be locked
 *
 * @return 0 if the lock happened, and -1 if something went wrong.
 */
int sem_down(semaphore_t *sem);

//=============================================================================
// Barrier Management
//=============================================================================

/**
 * @brief Initializes a new barrier.
 *
 * Initializes the barrier structure with a initial value and a empty queue.
 *
 * @param barrier Pointer for the barrier
 * @param num Number of tasks waiting this barrier to complete.
 *
 * @return 0 if successfuly created, and -1 if something went wrong.
 */
int barrier_init(barrier_t *barrier, int num);

/**
 * @brief Destroy a barrier
 *
 * Destroy the barrier passed by the pointer, and wake up all the tasks that are
 * waiting for this barrier. This tasks return from with a error code.
 *
 * @param barrier Pointer for the barrier to be destroyed
 *
 * @return 0 if successfuly destroyed, and -1 otherwise.
 */
int barrier_destroy(barrier_t *barrier);

/**
 * @brief Indicates that the task reached this barrier
 *
 * Indicates that the task reached this barrier. This call is a blocking one, if
 * the limit of the barrier was not reached.
 *
 * @param barrier Pointer for the barrier, that needs to be joined.
 *
 * @return 0 on success, and -1 otherwise.
 */
int barrier_join(barrier_t *barrier);

#endif // PPOS_H