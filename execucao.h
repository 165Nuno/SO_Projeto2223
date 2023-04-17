#include <stdlib.h>
#include <stdio.h>

typedef struct exec{ 
  int pid;          
  char *nome;
  long tempo;     
}*Exec;

Exec controiExec(int pid, long tempo, char *nome);