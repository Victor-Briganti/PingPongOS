// PingPongOS - PingPong Operating System
// Victor Briganti
// Versão 0.1 -- September 2024
// Test the implementation of the generic queue queue.c/queue.h.

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) &&              \
                        (!defined(__APPLE__) || !defined(__MACH__)))
#warning This code is developed to work in UNIX systems. It may not work as intended on other systems.
#endif

#include "lib/queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define N 100

// The structure "queueint_t" is going to be used with the functions in queue.c,
// it should be used by a casting with the "queue_t". This only works if the
// first fields are the same between the structures. This is true if we follow
// the 6.7.2.1 of the C99 standard:"Within a structure object, the non-bit-ﬁeld
// members and the units in which bit-ﬁelds reside have addresses that increase
// in the order in which they are declared.".

typedef struct queueint_t {
  struct queueint_t *prev; // ptr that is going to be casted to queue_t
  struct queueint_t *next; // ptr that is going to be casted to queue_t
  int index;
  // Other fields can be placed in here
} queueint_t;

//------------------------------------------------------------------------------
// Auxiliary Functions
//------------------------------------------------------------------------------

// Prints in the screen the elements in of the queue
void print_elem(void *ptr) {
  queueint_t *elem = ptr;

  if (!elem) {
    return;
  }

  elem->prev ? printf("%d", elem->prev->index) : printf("*");
  printf("<%d>", elem->index);
  elem->next ? printf("%d", elem->next->index) : printf("*");
}

// Returns 0 if the queue is correct, 1 otherwise
int check_queue(queueint_t *queue) {
  // Empty queue is always correct
  if (!queue) {
    return 0;
  }

  // Queue with only one element and correct
  if ((queue->next == queue) && (queue->prev == queue)) {
    return 0;
  }

  queueint_t *aux = queue;
  do {
    if (!(aux->next && (aux->next->prev == aux))) {
      printf("->next is wrong\n");
      return 1;
    }

    if (!(aux->prev && (aux->prev->next == aux))) {
      printf("->prev is wrong\n");
      return 1;
    }

    aux = aux->next;
  } while (aux != queue);

  return 0;
}

// Creates the elements of the queue
queueint_t *create_itens() {
  queueint_t *items = (queueint_t *)calloc(N, sizeof(queueint_t));

  for (int i = 0; i < N; i++) {
    items[i].index = i;
    items[i].prev = NULL;
    items[i].next = NULL;
  }

  return items;
}

//------------------------------------------------------------------------------
// Test Functions
//------------------------------------------------------------------------------

int queue_insert_test() {
  queueint_t *items = create_itens();

  queueint_t *queue0 = NULL;
  for (int i = 0; i < N; i++) {
    if (queue_size((queue_t *)queue0) != i) {
      printf("INSERT TEST: Incorrect queue size\n");
      free(items);
      return 1;
    }

    queue_append((queue_t **)&queue0, (queue_t *)&(items[i]));

    if (check_queue(queue0) < 0) {
      printf("INSERT TEST: Queue is incorrect\n");
      free(items);
      return 1;
    }
  }

  free(items);
  return 0;
}

int queue_order_test() {
  queueint_t *items = create_itens();

  queueint_t *queue0 = NULL;
  for (int i = 0; i < N; i++) {
    queue_append((queue_t **)&queue0, (queue_t *)&(items[i]));
  }

  // Check if the order is right
  int index = 0;
  queueint_t *aux = queue0;
  do {
    if (index != aux->index) {
      printf("Wrong position [%d] should be [%d]", aux->index, index);
      free(items);
      return 1;
    }

    aux = aux->next;
    index++;
  } while (aux != queue0);
  assert(index == N);

  int size = queue_size((queue_t *)queue0);
  if (size != N) {
    printf("Wrong queue size received [%d] should be [%d]", size, N);
    free(items);
    return 1;
  }

  free(items);
  return 0;
}

int queue_remove_first_test() {
  queueint_t *items = create_itens();

  queueint_t *queue0 = NULL;
  for (int i = 0; i < N; i++) {
    queue_append((queue_t **)&queue0, (queue_t *)&(items[i]));
  }

  int index = 0;
  while (index < N) {
    queueint_t *aux = queue0;
    queue_remove((queue_t **)&queue0, (queue_t *)aux);

    if (check_queue(queue0)) {
      printf("Queue is not correct\n");
      free(items);
      return 1;
    }

    if (aux->index != index) {
      printf("Order of elements is wrong, expected [%d] got [%d]\n", index,
             aux->index);
      free(items);
      return 1;
    }

    if (aux->prev != NULL) {
      printf("->prev is a NULL pointer");
      free(items);
      return 1;
    }

    if (aux->next != NULL) {
      printf("->next is a NULL pointer");
      free(items);
      return 1;
    }
    index++;
  }

  if (queue0 != NULL) {
    printf("Queue is not NULL");
    free(items);
    return 1;
  }

  free(items);
  return 0;
}

int queue_remove_second_test() {
  queueint_t *items = create_itens();

  queueint_t *queue0 = NULL;
  for (int i = 0; i < N; i++) {
    queue_append((queue_t **)&queue0, (queue_t *)&(items[i]));
  }

  int index = 0;
  while (index < N) {
    queueint_t *aux = queue0->next;
    queue_remove((queue_t **)&queue0, (queue_t *)aux);

    if (check_queue(queue0)) {
      printf("Queue is not correct\n");
      free(items);
      return 1;
    }

    if (aux->index != ((index + 1) % N)) {
      printf("Order of elements is wrong, expected [%d] got [%d]\n",
             ((index + 1) % N), aux->index);
      free(items);
      return 1;
    }

    if (aux->prev != NULL) {
      printf("->prev is a NULL pointer");
      free(items);
      return 1;
    }

    if (aux->next != NULL) {
      printf("->next is a NULL pointer");
      free(items);
      return 1;
    }
    index++;
  }

  if (queue0 != NULL) {
    printf("Queue is not NULL");
    free(items);
    return 1;
  }

  free(items);
  return 0;
}

int queue_remove_last_test() {
  queueint_t *items = create_itens();

  queueint_t *queue0 = NULL;
  for (int i = 0; i < N; i++) {
    queue_append((queue_t **)&queue0, (queue_t *)&(items[i]));
  }

  int index = 0;
  while (index < N) {
    queueint_t *aux = queue0->prev;
    queue_remove((queue_t **)&queue0, (queue_t *)aux);

    if (check_queue(queue0)) {
      printf("Queue is not correct\n");
      free(items);
      return 1;
    }

    if (aux->index + index != N - 1) {
      printf("Order of elements is wrong, expected [%d] got [%d]\n", N - 1,
             aux->index + index);
      free(items);
      return 1;
    }

    if (aux->prev != NULL) {
      printf("->prev is a NULL pointer");
      free(items);
      return 1;
    }

    if (aux->next != NULL) {
      printf("->next is a NULL pointer");
      free(items);
      return 1;
    }
    index++;
  }

  if (queue0 != NULL) {
    printf("Queue is not NULL");
    free(items);
    return 1;
  }

  free(items);
  return 0;
}

int queue_remove_random_test() {
  queueint_t *items = create_itens();

  queueint_t *queue0 = NULL;
  for (int i = 0; i < N; i++) {
    queue_append((queue_t **)&queue0, (queue_t *)&(items[i]));
  }

  int index = 0;
  while (queue0) {
    index = rand() % queue_size((queue_t *)queue0);
    queueint_t *aux = queue0;
    while (index) {
      index--;
      aux = aux->next;
    }

    queue_remove((queue_t **)&queue0, (queue_t *)aux);
  }

  if (queue0 != NULL) {
    printf("Queue is not NULL");
    free(items);
    return 1;
  }

  free(items);
  return 0;
}

int queue_invalid_insert() {
  queueint_t item0 = {.prev = NULL, .next = NULL, .index = 1};
  queueint_t item1 = {.prev = NULL, .next = NULL, .index = 1};

  queueint_t *queue0 = NULL;
  queueint_t *queue1 = NULL;

  queue_append((queue_t **)&queue0, (queue_t *)&item0);
  queue_append((queue_t **)&queue1, (queue_t *)&item1);

  queue_remove((queue_t **)&queue0, (queue_t *)&item1);

  if (queue0 != &item0) {
    printf("Invalid removal\n");
    return 1;
  }

  if (queue0->prev != &item0) {
    printf("Queue0 should be the only element\n");
    return 1;
  }

  if (queue0->next != &item0) {
    printf("Queue0 should be the only element\n");
    return 1;
  }

  if (queue1->prev != &item1) {
    printf("Queue1 should be the only element\n");
    return 1;
  }

  if (queue1->next != &item1) {
    printf("Queue1 should be the only element\n");
    return 1;
  }

  return 0;
}

int queue_remove_alone_insert() {
  queueint_t item0 = {.prev = NULL, .next = NULL, .index = 1};
  queueint_t item1 = {.prev = NULL, .next = NULL, .index = 1};

  queueint_t *queue0 = NULL;

  queue_append((queue_t **)&queue0, (queue_t *)&item0);
  queue_remove((queue_t **)&queue0, (queue_t *)&item1);

  if (queue0 != &item0) {
    printf("Invalid removal\n");
    return 1;
  }

  if (queue0->prev != &item0) {
    printf("Queue0 should be the only element\n");
    return 1;
  }

  if (queue0->next != &item0) {
    printf("Queue0 should be the only element\n");
    return 1;
  }

  if (item1.prev != NULL) {
    printf("Queue1 should be the only element\n");
    return 1;
  }

  if (item1.next != NULL) {
    printf("Queue1 should be the only element\n");
    return 1;
  }

  return 0;
}

int queue_insert_dup() {
  queueint_t item0 = {.prev = NULL, .next = NULL, .index = 1};
  queueint_t *queue0 = NULL;
  queue_append((queue_t **)&queue0, (queue_t *)&item0);

  if (queue_append((queue_t **)&queue0, (queue_t *)&item0) !=
      Q_ERR_ELEM_DUP_LIST) {
    printf("Invalid insertion of duplicated elements\n");
    return 1;
  }

  return 0;
}

int queue_insert_double_queue() {
  queueint_t item0 = {.prev = NULL, .next = NULL, .index = 1};
  queueint_t item1 = {.prev = NULL, .next = NULL, .index = 1};

  queueint_t *queue0 = NULL;
  queueint_t *queue1 = NULL;

  queue_append((queue_t **)&queue0, (queue_t *)&item0);
  queue_append((queue_t **)&queue1, (queue_t *)&item1);

  if (queue0 != &item0) {
    printf("Invalid insert in queue\n");
    return 1;
  }

  if (queue0->prev != &item0) {
    printf("Queue0 should be the only element\n");
    return 1;
  }

  if (queue0->next != &item0) {
    printf("Queue0 should be the only element\n");
    return 1;
  }

  if (queue1 != &item1) {
    printf("Invalid insert in queue\n");
    return 1;
  }

  if (queue1->prev != &item1) {
    printf("Queue1 should be the only element\n");
    return 1;
  }

  if (queue1->next != &item1) {
    printf("Queue1 should be the only element\n");
    return 1;
  }

  return 0;
}

//------------------------------------------------------------------------------
// Main Functions
//------------------------------------------------------------------------------

int main() {
  if (queue_insert_test()) {
    printf("TEST FAILED: queue_insert_test\n");
    return 1;
  }

  if (queue_order_test()) {
    printf("TEST FAILED: queue_order_test\n");
    return 1;
  }

  if (queue_remove_first_test()) {
    printf("TEST FAILED: queue_remove_first_test\n");
    return 1;
  }

  if (queue_remove_second_test()) {
    printf("TEST FAILED: queue_remove_second_test\n");
    return 1;
  }

  if (queue_remove_last_test()) {
    printf("TEST FAILED: queue_remove_last_test\n");
    return 1;
  }

  if (queue_remove_random_test()) {
    printf("TEST FAILED: queue_remove_random_test\n");
    return 1;
  }

  if (queue_invalid_insert()) {
    printf("TEST FAILED: queue_invalid_insert\n");
    return 1;
  }

  if (queue_remove_alone_insert()) {
    printf("TEST FAILED: queue_remove_alone_insert\n");
    return 1;
  }

  if (queue_insert_dup()) {
    printf("TEST FAILED: queue_insert_dup\n");
    return 1;
  }

  if (queue_insert_double_queue()) {
    printf("TEST FAILED: queue_insert_double_queue\n");
    return 1;
  }

  return 0;
}