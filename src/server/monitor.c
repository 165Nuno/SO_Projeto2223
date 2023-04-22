#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include "execucao.h"
#include "ListaLigadaExec.h"

#define MAX_BUFFER_SIZE 1024



int main(int argc, char *argv[]){
    long tempo;
    int n, pid, tam;
    char info[100], nome[100];
    char pasta[100];
    char ficheiro[114];


    sprintf(pasta, "../%s",argv[1]);
    // Serve para criar a pasta caso ela não esteja criada
    if (mkdir(pasta, 0777) == -1) {
        perror("Erro ao criar a pasta");
        exit(-1);
    }

	// Construção do FIFO
    // Este FIFO vai ser o FIFO utilizado por todos os clientes para fazer pedidos ao servidor
	int p = mkfifo("cliente_para_servidor",0660);
    if(p==-1){
        if(errno != EEXIST){// Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 1;
        }
    }

	// Abertura do FIFO
    // O servidor apenas lê deste FIFO
	int fin = open("cliente_para_servidor", O_RDONLY);
	if(fin == -1){
		perror("Erro ao abrir FIFO para ler e escrever");
		exit(-1);
	}

    int fout = open("cliente_para_servidor", O_WRONLY);
    if(fout == -1) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    // Lista que vai ter as execuções atuais
    LLEXEC l = initList();
    LLEXEC *lista = &l;

    while((n=read(fin, info, 6 * sizeof(char))) > 0){
        info[6] = '\0';
        printf("Info: %s\n", info);
        if(strcmp(info,"status") == 0){ // Status de todos os programas
            printf("Status!\n");
            // Recebe o nome do FIFO de escrita
            read(fin, &tam, sizeof(int));
            read(fin, info, tam * sizeof(char));

            // Um processo filho trata os pedidos "status"
            if(fork() == 0){
                execStatus(lista, info); // Envia a informação para o cliente
                exit(0);
            }
        }else if(strcmp(info, "execan") == 0){ // Pré execução de um programa
            printf("Execute antes!\n");
            // PID
            read(fin, &pid, sizeof(int));
            printf("[EXECAN DEBUG] PID: %d\n", pid);
            // Nome
            read(fin, &tam, sizeof(int));
            read(fin, &nome, tam * sizeof(char));
            printf("[EXECAN DEBUG] Nome: %s\n", nome);
            // TimeStamp
            read(fin, &tempo, sizeof(long));
            printf("[EXECAN DEBUG] TimeStamp: %li\n", tempo);

            Exec exe = constroiExec(pid, tempo, nome);
            insereElem(exe, lista);
        }else if(strcmp(info, "exefim") == 0){ // Pós execução de um programa
            printf("Execute depois!\n");

            // PID
            read(fin, &pid, sizeof(int));
            printf("[EXEFIM DEBUG] PID: %d\n", pid);

            // Nome
            read(fin,&tam,sizeof(int));
            read(fin,&nome,tam * sizeof(char));
            printf("[EXEFIM DEBUG] Nome: %s\n",nome);
            // TimeStamp
            read(fin, &tempo, sizeof(long));
            printf("[EXEFIM DEBUG] TimeStamp: %li\n", tempo);

            // Remove o programa da lista ligada que terminou a sua execução
            removeElem(pid, lista);

            // Iniciamos a construção do ficheiro
            sprintf(ficheiro,"./%s/%d", pasta, pid);
            
            FILE* file = fopen(ficheiro, "w");
            if (!file) {
                perror("Erro ao criar o arquivo");
                exit(-1);
            }

            fprintf(file, "Nome: %s\nTempo total: %lims\n", nome, tempo);
            fclose(file);

            // 
        }
    }

	return 0;
}
