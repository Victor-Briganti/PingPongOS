/*
 * PingPongOS - PingPong Operating System
 * Filename: pptask_manager.c
 * Description: ADT structure to manage task queues
 *
 * Author: Victor Briganti
 * Date: 2024-09-11
 * License: BSD 2
 */

/*
 * PingPongOS - PingPong Operating System
 * Filename: pptask_manager.h
 * Description: ADT structure to manage task queues
 *
 * Author: Victor Briganti
 * Date: 2024-09-11
 * License: BSD 2
 */

#include "adt/pptask_manager.h"
#include "debug/log.h"
#include "lib/queue.h"

#include <stdlib.h>

#define PRIORITY_AGE -1

//=============================================================================
// Private Functions
//=============================================================================
#ifdef DEBUG
/**
 * @brief Helper function to print the queue
 *
 * @param ptr Opaque pointer that is going to be casted to a task_t to be
 * printed.
 */
static void qtask_print(void *ptr) {
  task_t *task = (task_t *)ptr;

  if (task == NULL) {
    (void)fprintf(stderr, "(nil) ");
  }

  (void)fprintf(stderr, "%d ", task->tid);
}
#endif // DEBUG

static int qcompare(const void *ptr1, const void *ptr2) {
  task_t *elem = (task_t *)ptr1;
  task_t *queue = (task_t *)ptr2;

  return queue->current_priority - elem->initial_priority;
}

//=============================================================================
// Public Functions
//=============================================================================

TaskManager *task_manager_create() {
  TaskManager *manager = calloc(1, sizeof(TaskManager));
  return manager;
}

void task_manager_delete(TaskManager *manager) { free(manager); }

int task_manager_insert(TaskManager *manager, task_t *task) {
  if (manager == NULL) {
    log_error("%s received a NULL manager", __func__);
    return -1;
  }

  if (task == NULL) {
    log_error("%s received a NULL task", __func__);
    return -1;
  }

  log_debug("%s inserting task(%d) in queue", __func__, task->tid);
  task_manager_print(manager);
  if (queue_insert_inorder((queue_t **)&(manager->taskQueue), (queue_t *)task,
                           qcompare)) {
    log_error("%s could not insert task(%d) in queue", __func__, task->tid);
    return -1;
  }
  log_debug("%s queue after insertion", __func__);
  task_manager_print(manager);

  manager->count++;
  return 0;
}

int task_manager_remove(TaskManager *manager, task_t *task) {
  if (manager == NULL) {
    log_error("%s received a NULL manager", __func__);
    return -1;
  }

  if (task == NULL) {
    log_error("%s received a NULL task", __func__);
    return -1;
  }

  log_debug("%s removing task(%d) of the queue %p", __func__, task->tid);
  task_manager_print(manager);
  if (queue_remove((queue_t **)&(manager->taskQueue), (queue_t *)task) < 0) {
    log_error("%s could not remove task(%d) of the queue", __func__, task->tid);
    return -1;
  }
  log_debug("%s queue after insertion", __func__);
  task_manager_print(manager);

  task->current_priority = task->initial_priority;

  manager->count--;
  return 0;
}

void task_manager_aging(TaskManager *manager) {
  if (manager == NULL) {
    log_error("%s received a NULL manager", __func__);
    return;
  }

  task_t *aux = manager->taskQueue;
  if (!aux) {
    log_debug("%s queue is empty", __func__);
    return;
  }

  do {
    aux->current_priority += PRIORITY_AGE;
    aux = aux->next;
  } while (aux != manager->taskQueue);
}

#ifdef DEBUG
void task_manager_print(TaskManager *manager) {
  if (manager == NULL) {
    (void)fprintf(stderr, "NULL\n");
    return;
  }

  queue_print("taskQueue", (queue_t *)(manager->taskQueue), qtask_print);
}
#endif // DEBUG