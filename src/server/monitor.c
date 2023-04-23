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

/*
void exec_stats_time(char *command[], int argc,char* pasta){
    long contador = 0;
int i = 0;
const char delimiter[2] = " ";
char *commands[100];
FILE *fp;
char* ficheiro;


token = strtok(command, delimiter);
while (token != NULL) {
    commands[i] = token;    
    printf("Token: %s\n", token);
    token = strtok(NULL, delimiter);
    i++;
}
commands[i] = '\0';

for(i = 1; commands[i] != '\0';i++){
    sprintf(ficheiro,"../bin/%s/%s",pasta,commands[i]);
    fp = fopen(ficheiro, "r");
    if (fp == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1; // ou outro valor de retorno, dependendo da sua aplicação
    }

    char linha[1000];
    while (fgets(linha, sizeof(linha), fp) != NULL) {
        char *nome_ptr = strstr(linha, "Nome:");
        if (nome_ptr != NULL) {
            // Obtém o nome do artigo
            char nome[100];
            char *token = strtok(nome_ptr + strlen("Nome:"), "\n");
            strncpy(nome, token, sizeof(nome));

            // Obtém o tempo total do artigo
            char *tempo_total_ptr = strstr(linha, "Tempo total:");
            if (tempo_total_ptr != NULL) {
                char tempo_total_str[10];
                char *token = strtok(tempo_total_ptr + strlen("Tempo total:"), "ms");
                strncpy(tempo_total_str, token, sizeof(tempo_total_str));

                // Converte o tempo total para um número inteiro
                int tempo_total = atoi(tempo_total_str);

                // Imprime o nome do artigo e o tempo total
                printf("Nome: %s\nTempo total: %dms\n", nome, tempo_total);

                // Verifica se o nome corresponde ao arquivo desejado
                if (strcmp(commands[i], nome) == 0) {
                    contador += tempo_total;
                }
            }
        }
    }

    pclose(fp);
}

printf("Tempo total: %ldms\n", contador);
}
*/

void exec_command(char** str_array,int s,char* pasta,char *nomeFIFO){
    int i;
    int contador = 0;
    char nome[100];
    char* ficheiro;
    ficheiro = malloc(100);
    FILE *fp;

    int fout = open(nomeFIFO, O_WRONLY);
    if(fout == -1){
        perror("Houve um problema ao abrir o FIFO!");
    }
    for(i = 1; i < s; i++){
        sprintf(ficheiro,"../bin/%s/%s",pasta,str_array[i]);
        printf("ESTE È O FICHEIRO: %s\n",ficheiro);

        fp = fopen(ficheiro, "r");
        if (fp == NULL) {
        printf("Erro ao abrir o arquivo.\n"); // ou outro valor de retorno, dependendo da sua aplicação
        }

        char linha[1000];
    while (fgets(linha, sizeof(linha), fp) != NULL) {
        char *nome_ptr = strstr(linha, "Nome:");
        if (nome_ptr != NULL) {
            // Obtém o nome do artigo
            char *token = strtok(nome_ptr + strlen("Nome:"), "\n");
            strncpy(nome, token, sizeof(nome));
            
            // Tamanho da string nome
            int len = strlen(nome);

            // Ciclo que remove o primeiro elemento da string nome que é um espaço
            for (int i = 0; i < len; i++) {
                nome[i] = nome[i+1]; // move todos os caracteres para a esquerda
            }
            len--;
            // Atulizamos o comprimento da string

            // Compara se o nome do programa é igual ao nome do programa que estamos à procura
            if(strcmp(nome,str_array[0]) == 0){
                contador++;
            }
        }
    }

    // liberta memória
    free(ficheiro);
    // fecha o fp
    fclose(fp);

    }
    // Envia para o cliente o contador
    write(fout,&contador,sizeof(int));

}



void exec_time(char** str_array,int s,char* pasta,char *nomeFIFO){
    int i;
    int contador = 0;
    char nome[100];
    char* ficheiro;
    ficheiro = malloc(100);
    FILE *fp;

    int fout = open(nomeFIFO, O_WRONLY);
    if(fout == -1){
        perror("Houve um problema ao abrir o FIFO!");
    }


    for(i = 0; i < s; i++){
        sprintf(ficheiro,"../bin/%s/%s",pasta,str_array[i]);
        printf("ESTE È O FICHEIRO: %s\n",ficheiro);

        fp = fopen(ficheiro, "r");
        if (fp == NULL) {
        printf("Erro ao abrir o arquivo.\n"); // ou outro valor de retorno, dependendo da sua aplicação
        }

        char linha[1000];
        int encontrou_nome = 1;
    while (fgets(linha, sizeof(linha), fp) != NULL) {
        if (encontrou_nome == 0) {
            char *tempo_total_ptr = strstr(linha, "Tempo total:");
            if (tempo_total_ptr != NULL) {
                char tempo_total_str[10];
                char *token = strtok(tempo_total_ptr + strlen("Tempo total:"), "ms");
                strncpy(tempo_total_str, token, sizeof(tempo_total_str));

                // Converte o tempo total para um número inteiro
                int tempo_total = atoi(tempo_total_str);

                // Verifica se o nome corresponde ao arquivo desejado
                contador += tempo_total;

                // Reinicia a variável encontrou_nome para a próxima iteração do loop
                encontrou_nome = 1;
            }
        }

        char *nome_ptr = strstr(linha, "Nome:");
        if (nome_ptr != NULL) {
            // Obtém o nome do artigo
            char *token = strtok(nome_ptr + strlen("Nome:"), "\n");
            strncpy(nome, token, sizeof(nome));

            // Define a variável encontrou_nome como true para a próxima iteração do loop
            encontrou_nome = 0;
        }
    }

    free(ficheiro);
    fclose(fp);

    }

    write(fout,&contador,sizeof(int));
}

int main(int argc, char *argv[]){
    long tempo_antes;
    long tempo_depois;
    int n, pid, tam, s;
    char info[100], nome[100];
    char pasta[100];
    char ficheiro[114];
    char* f;


    sprintf(pasta, "../bin/%s",argv[1]);
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

        }else if(strcmp(info,"stcomd") == 0){
            printf("Stats-command\n");

            read(fin,&tam,sizeof(int));
            read(fin,info,tam * sizeof(char));
            printf("%s\n",info);

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
            printf("Stats-time\n");

            read(fin, &tam, sizeof(int));
            read(fin, info, tam * sizeof(char));
            printf("%s\n",info);

            f = info;

            
            read(fin,&s,sizeof(int));
            printf("TESTE: %d\n",s);


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
            read(fin, &tempo_antes, sizeof(long));
            printf("[EXECAN DEBUG] TimeStamp: %li\n", tempo_antes);

            Exec exe = constroiExec(pid, tempo_antes, nome);
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
            read(fin, &tempo_depois, sizeof(long));
            printf("[EXEFIM DEBUG] TimeStamp: %li\n", tempo_depois);

            long final = tempo_depois - tempo_antes;

            // Remove o programa da lista ligada que terminou a sua execução
            removeElem(pid, lista);

            // Iniciamos a construção do ficheiro
            sprintf(ficheiro,"../bin/%s/%d", pasta, pid);
            
            FILE* file = fopen(ficheiro, "w");
            if (!file) {
                perror("Erro ao criar o arquivo");
                exit(-1);
            }

            fprintf(file, "Nome: %s\nTempo total: %lims\n", nome, final);
            fclose(file);

            // 
        }
    }

	return 0;
}
