/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos.h
 * Description: Header for the public interface of the OS
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

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
