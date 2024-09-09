#include "queue.h"

#include "stdio.h"

int queue_size(queue_t *queue) {
  if (queue == NULL) {
    return 0;
  }

  queue_t *aux = queue;
  int size = 0;

  do {
    size++;
    aux = aux->next;
  } while (aux != queue);

  return size;
}

void queue_print(char *name, queue_t *queue, void print_elem(void *)) {
  printf("%s:", name);

  if (queue == NULL) {
    return;
  }

  queue_t *aux = queue;
  while (aux != queue) {
    print_elem(aux);
    aux = aux->next;
  }
}

int queue_append(queue_t **queue, queue_t *elem) {
  if (queue == NULL) {
    return QUEUE_NULL;
  }

  if (elem == NULL) {
    return QUEUE_ELEM_NULL;
  }

  // The queue does not have any element
  if (*queue == NULL) {
    elem->next = elem;
    elem->prev = elem;
    *queue = elem;
    return 0;
  }

  // Search through the list the elem passed
  queue_t *aux = *queue;
  while (aux != *queue) {
    if (aux == elem) {
      return QUEUE_ELEM_REPEATED;
    }

    aux = aux->next;
  }

  elem->next = *queue;
  elem->prev = (*queue)->prev;

  (*queue)->prev->next = elem;
  (*queue)->prev = elem;

  // If we have only one element the next pointer also needs to be updated
  if ((*queue)->next == *queue) {
    (*queue)->next = elem;
  }

  return 0;
}

// int queue_remove(queue_t **queue, queue_t *elem);
