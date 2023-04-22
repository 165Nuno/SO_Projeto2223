#include "execucao.h"

// >>> Função que constroi um Exec
Exec constroiExec(int pid, long tempo, char *nome){
    Exec exe = malloc(sizeof(struct exec));

    exe->pid = pid;
    exe->tempo = tempo;
    exe->nome = nome;

    return exe;
}