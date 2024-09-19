/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos_ipc.c
 * Description: Implementation of the Inter Process Comunication of the
 * PingPong OS
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#include "ppos.h"
#include "ppos_bkl.h"
#include "ppos_data.h"

#include <stdlib.h>

#define bkl_spinlock() while (bkl_lock())

//=============================================================================
// Semaphore Functions
//=============================================================================

int sem_init(semaphore_t *sem, int value) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state != SEM_CREATED) {
    return -1;
  }

  sem->state = SEM_INITALIZED;
  sem->lock = value;
  return 0;
}

int sem_destroy(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == SEM_FINISHED) {
    return -1;
  }

  bkl_spinlock();
  while (sem->queue) {
    task_awake(sem->queue, &(sem->queue));
  }

  sem->state = SEM_FINISHED;
  bkl_unlock();
  return 0;
}

int sem_up(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == SEM_FINISHED) {
    return -1;
  }

  if (sem->queue) {
    task_awake(sem->queue, &(sem->queue));
  }

  bkl_spinlock();
  sem->lock++;
  bkl_unlock();
  return 0;
}

int sem_down(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == SEM_FINISHED) {
    return -1;
  }

  while (!sem->lock && sem->state != SEM_FINISHED) {
    task_suspend(&(sem->queue));
  }

  bkl_spinlock();
  sem->lock--;
  bkl_unlock();
  return 0;
}

//=============================================================================
// Barrier Functions
//=============================================================================

int barrier_init(barrier_t *barrier, int num) {
  if (barrier == NULL) {
    return -1;
  }

  barrier->state = BAR_INITALIZED;
  barrier->queue = NULL;
  barrier->num_tasks = num;
  return 0;
}

int barrier_destroy(barrier_t *barrier) {
  if (barrier == NULL || barrier->state == BAR_FINISHED) {
    return -1;
  }

  while (barrier->queue) {
    task_awake(barrier->queue, &(barrier->queue));
  }

  barrier->state = BAR_FINISHED;
  return 0;
}

int barrier_join(barrier_t *barrier) {
  if (barrier == NULL || barrier->state == BAR_FINISHED) {
    return -1;
  }

  bkl_spinlock();
  barrier->num_tasks--;
  bkl_unlock();

  if (barrier->num_tasks <= 0) {
    while (barrier->queue) {
      task_awake(barrier->queue, &(barrier->queue));
      barrier->num_tasks++;
    }
  } else {
    task_suspend(&(barrier->queue));
  }

  return 0;
}
