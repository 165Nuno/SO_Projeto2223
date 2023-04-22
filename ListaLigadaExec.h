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

LLEXEC initList();

void insereElem(Exec exec, LLEXEC* lista);

void removeElem(int pid, LLEXEC* lista);

//void printaListaExe(LLEXEC* lista);

void execStatus(LLEXEC* lista, char *nomeFIFO);