#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>

long calcula_timestamp() {
    struct timeval tempo;
    int r = gettimeofday(&tempo, NULL);

    if (r == 0) {
        long tempo_ms = (tempo.tv_sec * 1000) + (tempo.tv_usec / 1000);
        return tempo_ms;
    } else {
        perror("ERRO >>> TimeStamp antes da execução");
        exit(-1);
    }
}


void executeBasicProgram(char *command[], int argc, int fout){
    int n = 0, pid_exec;
    char info[100];

    int pipefd[2];
    pipe(pipefd);

    if((pid_exec = fork()) == 0){ 
        // Estamos no processo [FILHO]
        close(pipefd[1]); // O filho só vai ler deste pipe
        read(pipefd[0], &n, sizeof(int));
        read(pipefd[0], info, n*sizeof(char));
        info[n-1] = '\0';
        close(pipefd[0]);

        if(strcmp(info, "ok") != 0){
            perror("ERRO >>> Não foi possível começar a execução do comando pretendido\n");
			exit(-1);
        }

        int res = execvp(command[0], command);
		if(res == -1){
			perror("ERRO >>> Não foi possível executar o comando pretendido\n");
			exit(-1);
		}
    }else{
    // Estamos no processo [PAI]
	close(pipefd[0]); // O pai só vai escrever para este pipe

	// -> Antes da execução
	// Avisa o servidor que é uma execução de um comando
	sprintf(info, "preexe");
	write(fout, info, strlen(info) * sizeof(char));
	// Escreve o PID do processo a executar o programa
	write(fout, &pid_exec, sizeof(int));
	// Nome do programa a executar
	n = strlen(command[0]);
	write(fout, &n, sizeof(int));
	write(fout, command[0], n * sizeof(char));
	// Timestamp
    long antes_ms = calcula_timestamp();
	write(fout, &antes_ms, sizeof(long));

	// Manda para o utilizador o PID (stdout)
	sprintf(info, "Running PID: %d\n", pid_exec);
	write(1, info, strlen(info) * sizeof(char));
    
    // Avisa o filho para começar a executar
    sprintf(info, "ok");
    n = strlen(info) + 1;
    write(pipefd[1], &n, sizeof(int));
    write(pipefd[1], info, n*sizeof(char));

	// Espera que o filho acabe de terminar a execução do programa
	wait(NULL);
	// -> Depois da execução
	// Inicia o processo pós-execução
	sprintf(info, "posexe");
	write(fout, info, strlen(info) * sizeof(char));
	// Escreve o PID do processo que terminou a execução
	write(fout, &pid_exec, sizeof(int));
    n = strlen(command[0]);
	write(fout, &n, sizeof(int));
	write(fout, command[0], n * sizeof(char));
	// Timestamp
    
    long fim_ms = calcula_timestamp();
    write(fout, &fim_ms, sizeof(long));

	// Tempo de execução (para o stdout):
	float execucao_tempo = fim_ms - antes_ms;
	sprintf(info, "Ended in: %.0fms\n", execucao_tempo);
	write(1,info,strlen(info)*sizeof(char));

    }
    
}


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
	int fout = open("cliente_servidor", O_WRONLY);
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
                executeBasicProgram(&argv[3], argc, fout);
            } else if(strcmp(argv[2], "-p") == 0){
                printf("Não conseguimos implementar pipeline\n");
                //executeProgramPipeLine(argv[3]);
            } else {
                sprintf(linha, "Erro Comando Inválido!\n");
                write(1, linha, strlen(linha) * sizeof(char));
            }
        } else if (strcmp(argv[1],"stats-time") == 0){
			sprintf(linha, "sttime");
            write(fout, linha, strlen(linha) * sizeof(char));

			tamanho = strlen(nome_fifo) + 1;
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