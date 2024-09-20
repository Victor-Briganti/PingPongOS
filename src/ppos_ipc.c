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
#include <string.h>

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

  if (barrier->state == BAR_FINISHED) {
    return -1;
  }

  return 0;
}

//=============================================================================
// Message Queue Functions
//=============================================================================

int mqueue_init(mqueue_t *queue, int max_msgs, int msg_size) {
  if (queue == NULL || max_msgs < 0) {
    return -1;
  }

  queue->state = MQE_INITALIZED;
  queue->index = 0;
  queue->num_msgs = 0;
  queue->max_msgs = max_msgs;
  queue->msg_size = (size_t)msg_size;

  if (sem_init(&(queue->sem_prod), max_msgs) < 0) {
    return -1;
  }

  if (sem_init(&(queue->sem_cons), 0) < 0) {
    return -1;
  }

  queue->msgs = calloc((size_t)max_msgs, (size_t)msg_size);

  return 0;
}

int mqueue_send(mqueue_t *queue, void *msg) {
  if (queue == NULL || queue->state == MQE_FINISHED) {
    goto error;
  }

  if (sem_down(&(queue->sem_prod)) < 0) {
    goto error;
  }

  if (queue->state == MQE_FINISHED) {
    goto error;
  }

  memcpy((char *)queue->msgs + ((size_t)queue->index * queue->msg_size), msg,
         queue->msg_size);

  queue->index = (queue->index + 1) % queue->max_msgs;

  if (sem_up(&(queue->sem_cons)) < 0) {
    goto error;
  }

  return 0;
error:
  return -1;
}

int mqueue_recv(mqueue_t *queue, void *msg) {
  if (queue == NULL || queue->state == MQE_FINISHED) {
    goto error;
  }

  if (sem_down(&(queue->sem_cons)) < 0) {
    goto error;
  }

  if (queue->state == MQE_FINISHED) {
    goto error;
  }

  queue->index--;
  if (queue->index < 0) {
    queue->index = queue->max_msgs - 1;
  }

  memcpy(msg, (char *)queue->msgs + ((size_t)queue->index * queue->msg_size),
         queue->msg_size);

  if (sem_up(&(queue->sem_prod)) < 0) {
    goto error;
  }

  if (queue->state == MQE_FINISHED) {
    goto error;
  }

  return 0;
error:
  return -1;
}

int mqueue_destroy(mqueue_t *queue) {
  if (queue == NULL || queue->state == MQE_FINISHED) {
    return -1;
  }

  queue->state = MQE_FINISHED;
  free(queue->msgs);

  if (sem_destroy(&(queue->sem_prod)) < 0) {
    return -1;
  }

  if (sem_destroy(&(queue->sem_cons)) < 0) {
    return -1;
  }

  return 0;
}

int mqueue_msgs(mqueue_t *queue) {
  if (queue == NULL || queue->state == MQE_FINISHED) {
    return -1;
  }

  return queue->num_msgs;
}
