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

/*
void executaProgramaPipeLine(char* command,int fout){
    int nrpipes = 0, i = 0;
    char *token;
    const char delimiter[2] = "|";

    printf("Command: %s\n", command);

    while(command[i] != '\0'){
        if(command[i] == '|'){
            nrpipes++;
        }
        i++;
    }

    i = 0;
    char *commands[nrpipes+1];

    token = strtok(command, delimiter);
    while (token != NULL) {
        commands[i] = token;    
        printf("Token: %s\n", token);
        token = strtok(NULL, delimiter);
        i++;
    }

    i=0;
    while(i<nrpipes+1){
        printf("%d: %s\n", i, commands[i]);
        i++;
    }

    // Inicialização dos pipes
    int pipes[nrpipes][2];
    for(i=0; i<nrpipes; i++){
        pipe(pipes[i]);
    }
        
    for(i = 0; i < nrpipes + 1; i++){
    // Cria o filho
    if(fork() == 0){
        int n = 0, j = 0;

        // conta o número de argumentos no comando atual
        while(commands[i][j] != '\0'){
            if(commands[i][j] == ' '){
                n++;
            }
            j++;
        }
        
        char *myCommand[n + 2]; // reserva espaço para os argumentos e NULL no final
        j = 0;

        // divide o comando atual em argumentos
        token = strtok(commands[i], " ");
        while (token != NULL) {
            myCommand[j] = token;
            token = strtok(NULL, " ");
            j++;
        }
        myCommand[j] = NULL; // coloca NULL no final do array

        if(i == 0){ // Primeiro filho
            close(pipes[0][0]); // fecha a leitura do primeiro pipe
            for(int j = 1; j < nrpipes; j++){ // fecha todas as outras pipes
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            dup2(pipes[0][1], 1); // redireciona a saída padrão para o primeiro pipe
            close(pipes[0][1]); // fecha a escrita do primeiro pipe
        }
        else if(i < nrpipes){ // Filhos do meio
            close(pipes[i-1][1]); // fecha a escrita da pipe anterior
            close(pipes[i][0]); // fecha a leitura da pipe atual
            dup2(pipes[i-1][0], 0); // redireciona a entrada padrão para a pipe anterior
            dup2(pipes[i][1], 1); // redireciona a saída padrão para a pipe atual
            close(pipes[i-1][0]); // fecha a leitura da pipe anterior
            close(pipes[i][1]); // fecha a escrita da pipe atual
        }
        else{ // Último filho
            close(pipes[i-1][1]); // fecha a escrita da última pipe
            dup2(pipes[i-1][0], 0); // redireciona a entrada padrão para a última pipe
            close(pipes[i-1][0]); // fecha a leitura da última pipe
        }
        
        // executa o comando atual
        execvp(myCommand[0], myCommand);
        perror("execvp failed"); // caso a execução falhe
        exit(1); // termina o processo filho com erro
    }
}

}
*/