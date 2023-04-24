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
	int tam;
	char linha[100];
	char nome_fifo[100];
	char auxiliar[132];
	int fin;

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
			fin = open(nome_fifo, O_RDONLY);
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
        } else if (strcmp(argv[1],"stats-time") == 0){
			sprintf(linha, "sttime");
            write(fout, linha, strlen(linha) * sizeof(char));

			tamanho = strlen(nome_fifo);
			write(fout, &tamanho, sizeof(int));
			write(fout, nome_fifo, tamanho * sizeof(char));


			// Calcular quantos pids temos
			int numero_pids = argc - 2;
			write(fout,&numero_pids,sizeof(int));

			// Envia para o servidor os PIDS
			for(int i = 2; i < argc; i++){
				tamanho = strlen(argv[i]);
				write(fout, &tamanho, sizeof(int));
				write(fout, argv[i], tamanho * sizeof(char));
			}

			fin = open(nome_fifo, O_RDONLY);
			if(fin == -1){
				perror("Erro ao abrir o FIFO para escrita e leitura.");
				exit(-1);
			}
			
			// A variável é tamanho, mas na verdade é o contador
			read(fin, &tamanho, sizeof(int));
			sprintf(auxiliar,"Total execution time is %dms\n",tamanho);
			write(1,auxiliar,strlen(auxiliar) * sizeof(char));

            
        }else if (strcmp(argv[1],"stats-uniq") == 0){
				sprintf(linha,"stuniq");
				write(fout,linha,strlen(linha) * sizeof(char));

				tamanho = strlen(nome_fifo);
				write(fout,&tamanho,sizeof(int));
				write(fout,nome_fifo,tamanho * sizeof(char));

				int numero_prog_pids = argc - 2;
				write(fout,&numero_prog_pids,sizeof(int));

				// Envia para o servidor os PIDS
				for(int i = 2; i < argc; i++){
					tamanho = strlen(argv[i]);
					write(fout, &tamanho, sizeof(int));
					write(fout, argv[i], tamanho * sizeof(char));
				}

				fin = open(nome_fifo, O_RDONLY);
				if(fin == -1){
					perror("Erro ao abrir o FIFO para escrita e leitura.");
					exit(-1);
				}
				
				read(fin,&tam,sizeof(int));

				for(int i = 0; i < tam; i++){
					read(fin,&tamanho,sizeof(int));
					read(fin,linha,tamanho * sizeof(char));
					sprintf(auxiliar,"%s",linha);
					write(1,auxiliar,strlen(auxiliar) * sizeof(char));
				}



			
		}else if (strcmp(argv[1],"stats-command") == 0){
				sprintf(linha,"stcomd");
				write(fout,linha,strlen(linha) * sizeof(char));

				tamanho = strlen(nome_fifo);
				write(fout,&tamanho,sizeof(int));
				write(fout,nome_fifo,tamanho * sizeof(char));

				int numero_prog_pids = argc - 2;
				write(fout,&numero_prog_pids,sizeof(int));

				// Envia para o servidor os PIDS
				for(int i = 2; i < argc; i++){
					tamanho = strlen(argv[i]);
					write(fout, &tamanho, sizeof(int));
					write(fout, argv[i], tamanho * sizeof(char));
				}

				fin = open(nome_fifo, O_RDONLY);
				if(fin == -1){
					perror("Erro ao abrir o FIFO para escrita e leitura.");
					exit(-1);
				}


				read(fin, &tamanho, sizeof(int));
				sprintf(auxiliar,"%s was executed %d times\n",argv[2],tamanho);
				write(1,auxiliar,strlen(auxiliar) * sizeof(char));


		}
        break;

	}
	return 0;
}