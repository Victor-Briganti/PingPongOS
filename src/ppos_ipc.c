#include "ppos.h"
#include "ppos_data.h"

#include <stdlib.h>

int sem_init(semaphore_t *sem, int value) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state != CREATED) {
    return -1;
  }

  sem->state = INITALIZED;
  sem->mutex = value;
  return 0;
}

int sem_destroy(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == FINISHED) {
    return -1;
  }

  while (sem->queue) {
    task_awake(sem->queue, &(sem->queue));
  }

  sem->state = FINISHED;
  return 0;
}

int sem_up(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == FINISHED) {
    return -1;
  }

  if (sem->queue) {
    task_awake(sem->queue, &(sem->queue));
  }

  sem->mutex++;
  return 0;
}

int sem_down(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == FINISHED) {
    return -1;
  }

  while (!sem->mutex && sem->state != FINISHED) {
    task_suspend(&(sem->queue));
  }

  sem->mutex--;
  return 0;
}