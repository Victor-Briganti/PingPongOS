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
#include "ppos_data.h"

#include <stdlib.h>
#include <string.h>

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

  (void)fprintf(stderr, "%d{%d} ", task->tid, task->current_priority);
}
#endif // DEBUG

//=============================================================================
// Public Functions
//=============================================================================

TaskManager *task_manager_create(char *name,
                                 int (*comp_func)(const void *ptr1,
                                                  const void *ptr2)) {
  if (!name) {
    log_error("received a NULL name");
    return NULL;
  }

  if (!comp_func) {
    log_error("received a NULL name");
    return NULL;
  }

  TaskManager *manager = calloc(1, sizeof(TaskManager));
  if (!manager) {
    log_error("could not allocate the manager");
    return NULL;
  }

  manager->name = strdup(name);
  if (!manager->name) {
    log_error("could not assign a name to the manager");
    return NULL;
  }

  manager->comp_func = comp_func;
  return manager;
}

void task_manager_delete(TaskManager *manager) {
  free(manager->name);
  free(manager);
}

int task_manager_insert(TaskManager *manager, task_t *task) {
  if (manager == NULL) {
    log_error("received a NULL manager");
    return -1;
  }

  if (task == NULL) {
    log_error("received a NULL task");
    return -1;
  }

  log_debug("inserting task(%d) in queue", task->tid);
  task_manager_print(manager);
  if (queue_insert_inorder((queue_t **)&(manager->taskQueue), (queue_t *)task,
                           manager->comp_func)) {
    log_error("could not insert task(%d) in queue", task->tid);
    return -1;
  }
  task_manager_print(manager);

  manager->count++;
  return 0;
}

int task_manager_remove(TaskManager *manager, task_t *task) {
  if (manager == NULL) {
    log_error("received a NULL manager");
    return -1;
  }

  if (task == NULL) {
    log_error("received a NULL task");
    return -1;
  }

  if (!manager->taskQueue) {
    log_debug("queue is empty");
    return -1;
  }

  log_debug("removing task(%d) of the queue %p", task->tid);
  task_manager_print(manager);
  if (queue_remove((queue_t **)&(manager->taskQueue), (queue_t *)task) < 0) {
    log_error("could not remove task(%d) of the queue", task->tid);
    return -1;
  }
  task_manager_print(manager);

  task->current_priority = task->initial_priority;
  manager->count--;
  return 0;
}

void task_manager_map(TaskManager *manager, void (*map_func)(void *ptr)) {
  if (manager == NULL) {
    log_error("received a NULL manager");
    return;
  }

  if (map_func == NULL) {
    log_error("received a NULL function");
    return;
  }

  if (!manager->taskQueue) {
    log_debug("queue is empty");
    return;
  }

  log_debug("mapping the queue");
  queue_map((queue_t *)(manager->taskQueue), map_func);
}

int task_manager_search(TaskManager *manager, task_t *task) {
  if (manager == NULL) {
    log_error("received a NULL manager");
    return -1;
  }

  if (task == NULL) {
    log_error("received a NULL task");
    return -1;
  }

  if (!manager->taskQueue) {
    log_debug("queue is empty");
    return -1;
  }

  task_t *aux = manager->taskQueue;
  do {
    if (aux == task) {
      return 0;
    }

    aux = aux->next;
  } while (aux != manager->taskQueue);

  return -1;
}

#ifdef DEBUG
void task_manager_print(TaskManager *manager) {
  if (manager == NULL) {
    (void)fprintf(stderr, "NULL\n");
    return;
  }

  if (!manager->taskQueue) {
    (void)fprintf(stderr, "%s: empty\n", manager->name);
    return;
  }

  (void)fprintf(stderr, "%s: ", manager->name);
  queue_map((queue_t *)(manager->taskQueue), qtask_print);
  (void)fprintf(stderr, "\n");
}
#endif // DEBUG