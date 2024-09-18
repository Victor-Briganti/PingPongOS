// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Testes de condição de corrida do semaforo

#include "ppos.h"
#include "ppos_data.h"
#include <stdio.h>

#define NUMTASKS (30)
#define NUMSTEPS (1000000)

task_t task[NUMTASKS];
semaphore_t s;
long int soma = 0;

// corpo da tarefa
void taskBody(void *id) {
  for (int i = 0; i < NUMSTEPS; i++) {
    sem_down(&s);
    soma += 1;
    sem_up(&s);
  }

  task_exit(0);
}

int main(int argc, char *argv[]) {
  printf("main: inicio\n");
  ppos_init();

  // inicializa semaforo em 0 (bloqueado)
  sem_init(&s, 0);

  printf("%d tarefas somando %d vezes cada, aguarde\n", NUMTASKS, NUMSTEPS);

  // cria tarefas
  for (int i = 0; i < NUMTASKS; i++) {
    task_init(&(task[i]), taskBody, "Task");
  }

  // espera um pouco e libera o semáforo
  task_sleep(20);
  sem_up(&s);

  // aguarda as tarefas encerrarem
  for (int i = 0; i < NUMTASKS; i++) {
    task_wait(&(task[i]));
  }

  // destroi o semaforo
  sem_destroy(&s);

  // verifica se a soma está correta
  if (soma == ((long)NUMSTEPS * NUMTASKS)) {
    printf("Soma deu %ld valor correto!\n", soma);
  } else {
    printf("Soma deu %ld, deveria ser %d\n", soma, NUMSTEPS * NUMTASKS);
  }

  printf("main: fim\n");
  task_exit(0);
}
