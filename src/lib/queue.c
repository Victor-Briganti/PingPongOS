#include "lib/queue.h"

#include "stdio.h"

//------------------------------------------------------------------------------
// Public Functions
//------------------------------------------------------------------------------

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
  (void)fprintf(stderr, "%s:", name);

  if (queue == NULL) {
    (void)fprintf(stderr, "\n");
    return;
  }

  queue_t *aux = queue;
  do {
    print_elem(aux);
    aux = aux->next;
  } while (aux != queue);
  (void)fprintf(stderr, "\n");
}

int queue_append(queue_t **queue, queue_t *elem) {
  if (queue == NULL) {
    return Q_ERR_NULL;
  }

  if (elem == NULL) {
    return Q_ERR_ELEM_NULL;
  }

  if (elem->next != NULL || elem->prev != NULL) {
    return Q_ERR_ELEM_DUP_LIST;
  }

  // The queue does not have any element
  if (*queue == NULL) {
    elem->next = elem;
    elem->prev = elem;
    *queue = elem;
    return 0;
  }

  elem->next = *queue;
  elem->prev = (*queue)->prev;

  (*queue)->prev->next = elem;
  (*queue)->prev = elem;

  return 0;
}

int queue_insert_inorder(queue_t **queue, queue_t *elem,
                         int (*compare)(const void *ptr1, const void *ptr2)) {
  if (queue == NULL) {
    return Q_ERR_NULL;
  }

  if (elem == NULL) {
    return Q_ERR_ELEM_NULL;
  }

  if (elem->next != NULL || elem->prev != NULL) {
    return Q_ERR_ELEM_DUP_LIST;
  }

  // The queue does not have any element
  if (*queue == NULL) {
    elem->next = elem;
    elem->prev = elem;
    *queue = elem;
    return 0;
  }

  // Search through the list the elem passed
  int resultComp = 0;
  queue_t *aux = *queue;
  do {
    resultComp = compare(elem, aux);
    if (resultComp < 0) {
      break;
    }

    aux = aux->next;
  } while (aux != *queue);

  if (aux == *queue && resultComp >= 0) {
    return queue_append(queue, elem);
  }

  if (aux == *queue && resultComp == 0) {
    elem->prev = *queue;
    elem->next = (*queue)->next;
    (*queue)->next->prev = elem;
    (*queue)->next = elem;
    return 0;
  }

  if (aux == *queue && resultComp < 0) {
    elem->next = *queue;
    elem->prev = (*queue)->prev;
    (*queue)->prev = elem;
    if ((*queue)->next == *queue) {
      (*queue)->next = elem;
    }
    *queue = elem;
    return 0;
  }

  elem->next = aux;
  elem->prev = aux->prev;
  aux->prev->next = elem;
  aux->prev = elem;

  return 0;
}

int queue_remove(queue_t **queue, queue_t *elem) {
  if (queue == NULL) {
    return Q_ERR_NULL;
  }

  if (*queue == NULL) {
    return Q_ERR_EMPTY;
  }

  if (elem == NULL) {
    return Q_ERR_ELEM_NULL;
  }

  // Search through the list the elem passed
  queue_t *aux = *queue;
  do {
    if (aux == elem) {

      // Single element
      if (aux->next == aux && aux->prev == aux) {
        (*queue) = NULL;
        aux->next = NULL;
        aux->prev = NULL;
        return 0;
      }

      // Remove element from the list
      aux->next->prev = aux->prev;
      aux->prev->next = aux->next;

      // Element is the same as the head
      if (aux == (*queue)) {
        queue_t *temp = aux->next;
        (*queue) = temp;
      }

      aux->next = NULL;
      aux->prev = NULL;

      return 0;
    }

    aux = aux->next;
  } while (aux != *queue);

  return Q_ERR_ELEM_NOT_FOUND;
}
