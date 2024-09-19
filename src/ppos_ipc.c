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

#define spinlock() while (bkl_lock())

int sem_init(semaphore_t *sem, int value) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state != CREATED) {
    return -1;
  }

  sem->state = INITALIZED;
  sem->lock = value;
  return 0;
}

int sem_destroy(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == FINISHED) {
    return -1;
  }

  bkl_lock();
  while (sem->queue) {
    task_awake(sem->queue, &(sem->queue));
  }

  sem->state = FINISHED;
  bkl_lock();
  return 0;
}

int sem_up(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == FINISHED) {
    return -1;
  }

  spinlock();
  if (sem->queue) {
    task_awake(sem->queue, &(sem->queue));
  }

  sem->lock++;
  bkl_unlock();
  return 0;
}

int sem_down(semaphore_t *sem) {
  if (sem == NULL) {
    return -1;
  }

  if (sem->state == FINISHED) {
    return -1;
  }

  spinlock();
  while (!sem->lock && sem->state != FINISHED) {
    task_suspend(&(sem->queue));
  }

  sem->lock--;
  bkl_unlock();
  return 0;
}