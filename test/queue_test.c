// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Version 1.4 -- Jan. 2022
// Tests for the queue.c/queue.h.

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) &&              \
                        (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este código foi planejado para ambientes UNIX (Linux, *BSD, MacOS). A compilação e execução em outros ambientes é responsabilidade do usuário.
#endif

#include "queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define N 100

// The structure "filaint_t" will be used with the functions queue.c with a
// casting to "queue_t". Following the section 6.7.2.1 of the C99 standard:
// "Within a structure object, the non-bit-field members and the units in
// which bit-fields reside have addresses that increase in the order
// in which they are declared.".

typedef struct filaint_t {
  struct filaint_t *prev; // ptr to be cast to queue_t
  struct filaint_t *next; // ptr to be cast to queue_t
  int id;
  // Other fields could be placed in here
} filaint_t;

filaint_t item[N];
filaint_t *fila0 = NULL;
filaint_t *aux = NULL;
filaint_t *final = NULL;
int ret = 0;

//------------------------------------------------------------------------------

// Prints the elements of queue (called by the function queue_print)
void print_elem(void *ptr) {
  filaint_t *elem = ptr;

  if (!elem) {
    return;
  }

  elem->prev ? printf("%d", elem->prev->id) : printf("*");
  printf("<%d>", elem->id);
  elem->next ? printf("%d", elem->next->id) : printf("*");
}

//------------------------------------------------------------------------------

// Returns 1 if the structure of the queue is correct, 0 if not
int fila_correta(filaint_t *fila) {
  filaint_t *aux = NULL;

  // A empty queue is always correct
  if (!fila) {
    return 1;
  }

  // Queue with only one element correct.
  if ((fila->next == fila) && (fila->prev == fila)) {
    return 1;
  }

  // Queue with only one element incorrect
  if ((fila->next == fila) || (fila->prev == fila)) {
    printf("[ERROR]: wrong pointers in queue with only one element\n");
    return 0;
  }

  // Queue with more than one element, walk and test all the pointers
  aux = fila;
  do {
    // Test the next pointer
    if (!(aux->next && (aux->next->prev == aux))) {
      printf("[ERROR]: pointer wrong ->next ou ->next->prev\n");
      return 0;
    }

    // Test prev pointer
    if (!(aux->prev && (aux->prev->next == aux))) {
      printf("[ERROR]: pointer wrong ->prev ou ->prev->prev\n");
      return 0;
    }

    aux = aux->next;
  } while (aux != fila);

  // Everything is ok
  return 1;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv, char **envp) {
  int i = 0;
  for (i = 0; i < N; i++) {
    item[i].id = i;
    item[i].prev = NULL;
    item[i].next = NULL;
  }

  // queue_append and queue_size
  // Teste: inserir N elemementos na fila e verificar a estrutura
  printf("Testando insercao de %d elementos...\n", N);
  fila0 = NULL;
  for (i = 0; i < N; i++) {
    assert(queue_size((queue_t *)fila0) == i);
    queue_append((queue_t **)&fila0, (queue_t *)&item[i]);
    assert(fila_correta(fila0));
  }

  // Teste: contar o numero de elementos na fila e verificar a ordem
  // dos elementos inseridos
  printf("Testando tamanho da fila e ordem dos %d elementos...\n", N);
  aux = fila0;

  i = 0;
  do {
    assert(i == aux->id); // testa posição do elemento i
    i++;
    aux = aux->next;
  } while (aux != fila0);

  assert(i == N);

  assert(queue_size((queue_t *)fila0) == N);

  printf("Testes de insercao funcionaram!\n");

  return 0;
}
