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


// >>> Função responsável pelo o stats-uniq <<<
void exec_uniq(char** str_array, int s, char* pasta, char* nomeFIFO) {
    int i;
    int tam;
    char* ficheiro;
    char info[100];
    ficheiro = malloc(100);

    // Array auxiliar para armazenar nomes únicos
    char** nomes = malloc(s * sizeof(char*));
    for (i = 0; i < s; i++) {
        nomes[i] = malloc(100 * sizeof(char));
    }

    int num_nomes = 0;

    int fout = open(nomeFIFO, O_WRONLY);
    if (fout == -1) {
        perror("Houve um problema ao abrir o FIFO!");
    }
    for (i = 0; i < s; i++) {
        sprintf(ficheiro,"../bin/%s/%s",pasta,str_array[i]);
        printf("DEBUG >>> ESTE É O FICHEIRO: %s\n",ficheiro);

        int fd = open(ficheiro, O_RDONLY);
        if (fd == -1) {
            printf("Erro ao abrir o arquivo.\n"); // ou outro valor de retorno, dependendo da sua aplicação
            continue;
        }

        char buffer[1024];
        int bytes_lidos;

         while ((bytes_lidos = read(fd, buffer, 1024)) > 0) {
            buffer[bytes_lidos] = '\0';

            char comando[1024];
            char *token = strtok(buffer, "\n");
            strcpy(comando, token);

            if (num_nomes == 0) {
                    strcpy(nomes[0], comando);
                    num_nomes++;
            } else {
                    int nome_existente = 0;
                    for (int j = 0; j < num_nomes; j++) {
                        if (strcmp(comando, nomes[j]) == 0) {
                        nome_existente = 1;
                        break;
                        }  
                    }

                    if (nome_existente == 0) {
                    // Adiciona o nome do programa ao array de nomes únicos
                    strcpy(nomes[num_nomes], comando);
                    num_nomes++;
                    }

            }


        }

    close(fd);
    }
    // Envio para o cliente o número de nomes unicos
    write(fout,&num_nomes,sizeof(int));

    // Envio todos os nomes unicos
    for(i = 0; i < num_nomes; i++){
        sprintf(info, "%s\n", nomes[i]);
        printf("DEBUG >>> %s\n",nomes[i]);
        tam = strlen(info);
        write(fout,&tam,sizeof(int));
        write(fout,info,strlen(info) * sizeof(char));
    }


    // liberta memória
    free(ficheiro);
    for (i = 0; i < s; i++) {
        free(nomes[i]);
    }
    free(nomes);
}



// >>> Função responsável pelo o stats-command <<<

void exec_command(char** str_array,int s,char* pasta,char *nomeFIFO){
    int i;
    int contador = 0;
    char* ficheiro;
    ficheiro = malloc(100);

    int fout = open(nomeFIFO, O_WRONLY);
    if(fout == -1){
        perror("Houve um problema ao abrir o FIFO!");
    }
    for(i = 1; i < s; i++){
        sprintf(ficheiro,"../bin/%s/%s",pasta,str_array[i]);
        printf("DEBUG >>> ESTE É O FICHEIRO: %s\n",ficheiro);

        int fd = open(ficheiro, O_RDONLY);
        if (fd == -1) {
            printf("Erro ao abrir o arquivo.\n"); // ou outro valor de retorno, dependendo da sua aplicação
            continue;
        }

        char buffer[1024];
        int bytes_lidos;

         while ((bytes_lidos = read(fd, buffer, 1024)) > 0) {
            buffer[bytes_lidos] = '\0';

            char comando[1024];
            char *token = strtok(buffer, "\n");
            strcpy(comando, token);

            if (strcmp(comando,str_array[0]) == 0){
                contador++;
            }

        }

    close(fd);
    }
    printf("DEBUG >>> contador do stats-command: %d\n",contador);
    // Envia para o cliente o contador
    write(fout,&contador,sizeof(int));

}

// >>> Função responsável pelo o stats-time <<<

void exec_time(char** str_array,int s,char* pasta,char *nomeFIFO){
    int i;
    int contador = 0;
    char* ficheiro;
    ficheiro = malloc(100);
        
    for(i = 0; i < s; i++){
        sprintf(ficheiro,"../bin/%s/%s",pasta,str_array[i]);
        printf("DEBUG >>> ESTE É O FICHEIRO: %s\n",ficheiro);

        int fd = open(ficheiro, O_RDONLY);
        if (fd == -1) {
            printf("Erro ao abrir o arquivo.\n"); // ou outro valor de retorno, dependendo da sua aplicação
            continue;
        }

        char buffer[1024];
        int bytes_lidos;

        while ((bytes_lidos = read(fd, buffer, 1024)) > 0) {
            buffer[bytes_lidos] = '\0';
            char *token = strtok(buffer, "\n"); // divide a string em "sleep" e "30"
            token = strtok(NULL, "\n"); // obtém o próximo token, que é "30"
            int atual = atoi(token);
            contador += atual;
        }

        close(fd);
        printf("CONTADOR : %d",contador);
    }
    
    int fout = open(nomeFIFO, O_WRONLY);
    if(fout == -1){
        perror("Houve um problema ao abrir o FIFO!");
    }

    write(fout,&contador,sizeof(int));
}

// >>> Main do Monitor <<<

int main(int argc, char *argv[]){
    long tempo_antes;
    long tempo_depois;
    int n, pid, tam, s;
    char info[100], nome[100];
    char pasta[100];
    char ficheiro[114];
    char* f;


    sprintf(pasta, "../tmp/%s",argv[1]);

	// Construção do FIFO
    // Este FIFO vai ser o FIFO utilizado por todos os clientes para fazer pedidos ao servidor
	int p = mkfifo("cliente_servidor",0660);
    if(p==-1){
        if(errno != EEXIST){// Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 1;
        }
    }

	// Abertura do FIFO
    // O servidor apenas lê deste FIFO
	int fin = open("cliente_servidor", O_RDONLY);
	if(fin == -1){
		perror("Erro ao abrir FIFO para ler e escrever");
		exit(-1);
	}

    int fout = open("cliente_servidor", O_WRONLY);
    if(fout == -1) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    // Lista que vai ter as execuções atuais
    LLEXEC l = inicializa_lista();
    LLEXEC *lista = &l;

    while((n=read(fin, info, 6 * sizeof(char))) > 0){
        info[6] = '\0';
        printf("Informação lida: %s\n", info);
        if(strcmp(info,"status") == 0){ // Status de todos os programas
            printf("Status!\n");
            // Recebe o nome do FIFO de escrita
            read(fin, &tam, sizeof(int));
            read(fin, info, tam * sizeof(char));
            
            // Um processo filho trata os pedidos "status"
            if(fork() == 0){
                execuca_status(lista, info); // Envia a informação para o cliente
                exit(0);
            }

        }else if(strcmp(info,"stuniq") == 0){
            printf("DEBUG >>> Stats-uniq\n");

            read(fin,&tam,sizeof(int));
            read(fin,info,tam*sizeof(char));

            f = info;

            read(fin,&s,sizeof(int));

            char** str_uniq = malloc(s * sizeof(char *));

            for(int i = 0; i < s; i++){
                read(fin,&tam,sizeof(int));
                char* info = malloc((tam + 1) * sizeof(char));
                read(fin,info,tam * sizeof(char));
                info[tam] = '\0';
                str_uniq[i] = strdup(info);
            }


            if (fork() == 0){
                exec_uniq(str_uniq,s,pasta,f);
                exit(0);
            }


        }else if(strcmp(info,"stcomd") == 0){
            printf("DEBUG >>> Stats-command\n");

            read(fin,&tam,sizeof(int));
            read(fin,info,tam * sizeof(char));

            f = info;

            read(fin,&s,sizeof(int));

            char** str_command = malloc(s * sizeof(char*));

            for(int i = 0; i < s; i++){
                read(fin,&tam,sizeof(int));
                char* info = malloc((tam + 1) * sizeof(char));
                read(fin,info,tam * sizeof(char));
                info[tam] = '\0';
                str_command[i] = strdup(info);
            }

            if (fork() == 0){
                exec_command(str_command,s,pasta,f);
                exit(0);
            }

        }else if(strcmp(info,"sttime") == 0){
            printf("DEBUG >>> Stats-time\n");

            read(fin, &tam, sizeof(int));
            read(fin, info, tam * sizeof(char));

            f = info;

            
            read(fin,&s,sizeof(int));


            char** str_array = malloc(s * sizeof(char*));

            for(int i = 0; i < s; i++){
                read(fin,&tam,sizeof(int));
                char* info = malloc((tam + 1) * sizeof(char));
                read(fin,info,tam * sizeof(char));
                info[tam] = '\0';
                str_array[i] = strdup(info);
            }

            if (fork() == 0){
                exec_time(str_array,s,pasta,f);
                exit(0);
            }

  
        }else if(strcmp(info, "preexe") == 0){ // Pré execução de um programa
            printf("DEBUG >>> Pré-Execução!!!!\n");
            // PID
            read(fin, &pid, sizeof(int));
            printf("DEBUG >>> Pré-execução - PID: %d\n", pid);
            // Nome
            read(fin, &tam, sizeof(int));
            read(fin, &nome, tam * sizeof(char));
            printf("DEBUG >>> Pré-execução - Nome: %s\n", nome);
            // TimeStamp
            read(fin, &tempo_antes, sizeof(long));
            printf("DEBUG >>> Pré-execução - TimeStamp: %li\n", tempo_antes);

            Exec exe = constroi_execucao(pid, tempo_antes, nome);
            insere_elemento(exe, lista);
        }else if(strcmp(info, "posexe") == 0){ // Pós execução de um programa
            printf("DEBUG >>> Pós-Execução!!!!\n");

            // PID
            read(fin, &pid, sizeof(int));
            printf("DEBUG >>> Pós-execução - PID: %d\n", pid);

            // Nome
            read(fin,&tam,sizeof(int));
            read(fin,&nome,tam * sizeof(char));
            printf("DEBUG >>> Pós-execução - Nome: %s\n",nome);
            // TimeStamp
            read(fin, &tempo_depois, sizeof(long));
            printf("DEBUG >>> Pós-execução - TimeStamp: %li\n", tempo_depois);

            long final = tempo_depois - tempo_antes;

            // Remove o programa da lista ligada que terminou a sua execução
            remove_elemento(pid, lista);

            // Iniciamos a construção do ficheiro
            sprintf(ficheiro,"../tmp/%s/%d", pasta, pid);
            
            // Escrever para o ficheiro
             int control_char_pos = strcspn(nome, "\x7F");
            if (control_char_pos != strlen(nome)) {
            nome[control_char_pos] = '\0';
            }


            char informacao[200];
            //fprintf(file, "Nome do programa: %s\nTempo total de execucao: %lims\n", nome, final);
            sprintf(informacao,"%s\n%li", nome, final);
            //fclose(file);

            int fd = open(ficheiro, O_WRONLY | O_CREAT | O_TRUNC, 0644); // abre o arquivo para escrita
            if (fd == -1) {
                perror("open");
                exit(-1);
            }

            size_t length = strlen(informacao); // tamanho da string
            ssize_t bytes_written = write(fd, informacao, length); // escreve a string no arquivo
            if (bytes_written == -1) {
                perror("write");
                exit(-1);
            }
            close(fd);
        }
    }

	return 0;
}
