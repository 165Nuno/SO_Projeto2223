#ifndef EXECUCAO_H
#define EXECUCAO_H
#include <stdlib.h>
#include <stdio.h>

typedef struct exec{ 
  int pid;          
  char *nome;
  long tempo; 
}*Exec;

Exec constroi_execucao(int pid, long tempo, char *nome);


#endif /* EXECUCAO_H */