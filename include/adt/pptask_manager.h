/*
 * PingPongOS - PingPong Operating System
 * Filename: pptask_manager.h
 * Description: ADT structure to manage task queues
 *
 * Author: Victor Briganti
 * Date: 2024-09-11
 * License: BSD 2
 */

#ifndef PPTASK_MANAGER_H
#define PPTASK_MANAGER_H

#include "ppos_data.h"

typedef struct {
  task_t *taskQueue;
  int count;
} TaskManager;

/**
 * @brief Creates a new Task Manager structure
 *
 * Allocates a new task manager structure and returns it.
 *
 * @return The new allocated structure, or NULL if something went wrong
 */
TaskManager *task_manager_create();

/**
 * @brief Deletes a Task Manager structure
 *
 * Deallocates the task manager structure.
 *
 * @param manager Pointer for the task manager structure that is going to be
 * deallocated
 */
void task_manager_delete(TaskManager *manager);

/**
 * @brief Inserts a task into the task queue.
 *
 * This operation appends the task into the task queue.
 *
 * @param manager Pointer for the Task Manager
 * @param task The task that is going to be inserted
 *
 * @return 0 if the task could be inserted, or 0> otherwise.
 */
int task_manager_insert(TaskManager *manager, task_t *task);

/**
 * @brief Remove a task of the task queue.
 *
 * @param manager Pointer for the Task Manager
 * @param task The task that is going to be removed.
 *
 * @return 0 if the task could be inserted, or 0> otherwise.
 */
int task_manager_remove(TaskManager *manager, task_t *task);

#ifdef DEBUG
/**
 * @brief Prints the queue inside the task manager
 *
 * @param manager Pointer for the Task Manager
 */
void task_manager_print(TaskManager *manager);
#else
#define task_manager_print(...)
#endif

#endif // PPTASK_MANAGER_H
