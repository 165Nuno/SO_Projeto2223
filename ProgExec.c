#include "ProgExec.h"

void executeBasicProgram(char *command[], int argc, int fout){
    struct timeval antes, fim;
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

        if(strcmp(info, "continua") != 0){
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

    }
    close(pipefd[0]); // O pai só vai escrever para este pipe

	// -> Antes da execução
	// Avisa o servidor que é uma execução de um comando
	sprintf(info, "execan");
	write(fout, info, strlen(info) * sizeof(char));
	// Escreve o PID do processo a executar o programa
	write(fout, &pid_exec, sizeof(int));
	// Nome do programa a executar
	n = strlen(command[0]);
	write(fout, &n, sizeof(int));
	write(fout, command[0], n * sizeof(char));
	// Timestamp
    int r = gettimeofday(&antes, NULL); // Acede ao antes e preenche os segundos e os microsegundos
    long antes_ms;

    if (r == 0) {
        antes_ms = (antes.tv_sec * 1000) + (antes.tv_usec / 1000); // Acedemos aos segundos e microsegundos da struct antes
        write(fout, &antes_ms, sizeof(long));
    } else {
        perror("ERRO >>> TimeStamp antes da execução");
        exit(-1);
    }
	// Manda para o utilizador o PID (stdout)
	sprintf(info, "Running PID: %d\n", pid_exec);
	write(1, info, strlen(info) * sizeof(char));
	// <-
    
    // Avisa o filho para começar a executar
    sprintf(info, "continua");
    n = strlen(info) + 1;
    write(pipefd[1], &n, sizeof(int));
    write(pipefd[1], info, n*sizeof(char));

	// Espera que o filho acabe de terminar a execução do programa
	wait(NULL);
	// -> Depois da execução
	// Inicia o processo pós-execução
	sprintf(info, "exefim");
	write(fout, info, strlen(info) * sizeof(char));
	// Escreve o PID do processo que terminou a execução
	write(fout, &pid_exec, sizeof(int));
    n = strlen(command[0]);
	write(fout, &n, sizeof(int));
	write(fout, command[0], n * sizeof(char));
	// Timestamp
    r = gettimeofday(&fim, NULL); // Acede ao antes e preenche os segundos e os microsegundos
    long fim_ms;
    if (r == 0) {
        fim_ms = (fim.tv_sec * 1000) + (fim.tv_usec / 1000); // Acedemos aos segundos e microsegundos da struct fim
        write(fout, &fim_ms, sizeof(long));
    } else {
        perror("ERRO >>> TimeStamp fim da execução");
        exit(-1);
    }
	// Tempo de execução (para o stdout):
	float execucao_tempo = fim_ms - antes_ms;
	sprintf(info, "Ended in: %.0fms\n", execucao_tempo);
	write(1,info,strlen(info)*sizeof(char));
	// <-
}