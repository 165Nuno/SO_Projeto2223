#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "execucao.h" 


typedef struct lligadaexecucao{
    Exec elem;
    struct lligadaexecucao *prox;
}*LLEXEC;

LLEXEC inicializa_lista();

void insere_elemento(Exec exec, LLEXEC* lista);

void remove_elemento(int pid, LLEXEC* lista);

void execuca_status(LLEXEC* lista, char *nomeFIFO);