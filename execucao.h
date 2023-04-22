#ifndef EXECUCAO_H
#define EXECUCAO_H
#include <stdlib.h>
#include <stdio.h>

typedef struct exec{ 
  int pid;          
  char *nome;
  long tempo; 
}*Exec;

Exec constroiExec(int pid, long tempo, char *nome);

void printaExec(Exec exe);

#endif /* EXECUCAO_H */