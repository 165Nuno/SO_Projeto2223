#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "ProgExec.h"


int main(int argc, char *argv[]){
	int pid;
	int tamanho;
	char linha[100];
	char nome_fifo[100];

	// Abertura do FIFO de escrita
	// Todos os clientes enviam os seus pedidos ao servidor por este FIFO
	int fout = open("cliente_para_servidor", O_WRONLY);
	if(fout == -1){
		perror("Erro ao abrir o FIFO para escrita e leitura.");
		exit(-1);
	}

	// Criação do FIFO de leitura
	pid = getpid();
	sprintf(nome_fifo, "fifo_%d", pid);
	int p = mkfifo(nome_fifo, 0660);
	if(p==-1){
        if(errno != EEXIST){// Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            exit(-1);
        }
    }

switch(argc) {
	// Caso em que só temos ./tracer que corresponde a um comando inválido
    case 1:
        sprintf(linha, "Erro Comando Inválido!\n");
        write(1, linha, strlen(linha) * sizeof(char));
        break;
	// Caso em que recebemos ./tracer status, se for igual a status dá erro de comando inválido	
    case 2:
        if(strcmp(argv[1], "status") == 0){
            sprintf(linha, "status");
            write(fout, linha, strlen(linha) * sizeof(char));
            
			// Manda o nome do FIFO para o servidor enviar as respostas
			tamanho = strlen(nome_fifo);
			write(fout, &tamanho, sizeof(int));
			write(fout, nome_fifo, strlen(nome_fifo) * sizeof(char));
		
			// Abertura do FIFO de leitura
			int fin = open(nome_fifo, O_RDONLY);
			if(fin == -1){
				perror("Erro ao abrir o FIFO para escrita e leitura.");
				exit(-1);
			}

			// Número de programas
			read(fin, &tamanho, sizeof(int));
			int nr = tamanho;

			if(nr == 0){  // Se o número de Programas for = 0, então receberá a linha do Servidor a dizer que não existem programas em execucao
				read(fin, &tamanho, sizeof(int));
				read(fin, linha, tamanho * sizeof(char));
				write(1, linha, tamanho * sizeof(char));
			}else{ // Caso seja != 0, então receber do servidor cada programa em execucao e mostrará ao utilizador
				for(int i=1; i<nr; i++){
					read(fin, &tamanho, sizeof(int));
					read(fin, linha, tamanho * sizeof(char));
					write(1, linha, tamanho * sizeof(char));
				}
			}
        } else {
            sprintf(linha, "Erro Comando Inválido!\n");
            write(1, linha, strlen(linha) * sizeof(char));
        }
        break;

	// Casos em que o argc > 2, casos dos executes
    default:
	    if(strcmp(argv[1], "execute") == 0){
            if(strcmp(argv[2], "-u") == 0){
                printf("Execução Básica de Programas\n");
                executeBasicProgram(&argv[3], argc, fout);
            } else if(strcmp(argv[2], "-p") == 0){
                printf("Pipeline de programas!\n");
                //executeProgramPipeLine(argv[3]);
            } else {
                sprintf(linha, "Erro Comando Inválido!\n");
                write(1, linha, strlen(linha) * sizeof(char));
            }
        } else {
            sprintf(linha, "Erro Comando Inválido!\n");
            write(1, linha, strlen(linha) * sizeof(char));
        }
        break;

	}
	return 0;
}