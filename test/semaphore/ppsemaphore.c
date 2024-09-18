// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste do escalonador por prioridades dinâmicas

#include "ppos.h"
#include <stdio.h>

task_t a1, a2, b1, b2;
semaphore_t s1, s2;

// corpo da thread A
void TaskA(void *arg) {
  for (int i = 0; i < 10; i++) {
    sem_down(&s1);
    printf("%s zig (%d)\n", (char *)arg, i);
    task_sleep(1000);
    sem_up(&s2);
  }

  task_exit(0);
}

// corpo da thread B
void TaskB(void *arg) {

  for (int i = 0; i < 10; i++) {
    sem_down(&s2);
    printf("%s zag (%d)\n", (char *)arg, i);
    task_sleep(1000);
    sem_up(&s1);
  }

  task_exit(0);
}

int main(int argc, char *argv[]) {
  printf("main: inicio\n");
  ppos_init();

  // cria semaforos
  sem_init(&s1, 1);
  sem_init(&s2, 0);

  // cria tarefas
  task_init(&a1, TaskA, "A1");
  task_init(&a2, TaskA, "    A2");
  task_init(&b1, TaskB, "                B1");
  task_init(&b2, TaskB, "                        B2");

  // aguarda a1 encerrar
  task_wait(&a1);

  // destroi semaforos
  sem_destroy(&s1);
  sem_destroy(&s2);

  // aguarda a2, b1 e b2 encerram
  task_wait(&a2);
  task_wait(&b1);
  task_wait(&b2);

  printf("main: fim\n");
  task_exit(0);
}
