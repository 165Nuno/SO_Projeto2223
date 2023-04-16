#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>








int main(int argc, char* argv[]){
    char pid[1024];
    char linha[1024];
    int fifo_fd, fifo_pid;
    sprintf(pid,"%d",getpid()); // a string pid fica com o pid processo atual
    sprintf(linha,"%d",getpid()); // a linha fica com o pid processo atual
    int size, i;
    int bytes_read;
    char buf[1024];

    char info[100]; // String para utilizar com auxiliar


    // Quando recebe apenas 1 argumento -> mostrar no ecrã as opções
    if(argc == 1 || (strcmp(argv[1], "execute") != 0 && strcmp(argv[1],"status") != 0)){
        sprintf(info,"Erro comando inválido!\n");
        write(1,info,strlen(info));
    }else if(strcmp(argv[1], "execute") == 0 && strcmp(argv[2], "-u") == 0){ // Execução de um comando básico
        sprintf(info,"VAI EXEUCTAR O EXECUTE!\n");
        write(1,info,strlen(info));
    }else if(strcmp(argv[1], "execute") == 0 && strcmp(argv[2], "-p") == 0){ // Execução de uma pipeline de programas
		printf("Pipeline de programas!\n");
    }else if(strcmp(argv[1], "status") == 0){
        sprintf(info,"Vamos executa o status!\n");
        write(1,info,strlen(info));
    }


    // Criei o FIFO PID (que tem associado o número do pid do processo atual)
    int xd = mkfifo(pid,0666);
    if (xd == -1){
        perror("Erro ao criar o FIFO do PID");
        return -1;
    }

    // Tou abrir o FIFOCS (que é criado no Servidor), lá o fifo_fd só lê , aqui ele só escreve
    if ((fifo_fd = open("fifoCS",O_WRONLY)) == -1) {
        perror("Erro ao abrir o fifo do Servidor para escrita");
        exit(1);
    }

    // Isto serve para juntar o pid com o resto daquilo que tem de executar
    if(argc > 0){
    strcat(linha," ");
    for(i = 1; i<argc-1; i++){
        strcat(linha,argv[i]);
        strcat(linha," ");
    }
    strcat(linha,argv[argc-1]);
    strcat(linha,"\n");
    }


    size = strlen(linha);

       if ( write (fifo_fd, &linha, size) == -1) {
           perror("Write");
       }


    // O Cliente usa o fifo_pid para ler do fifo pid
    if ((fifo_pid = open(pid,O_RDONLY)) == -1) {
        perror("Erro ao abrir fifo de leitura pid");
    } 
    // Escreve tudo o que lê do fifo_pid
    while((bytes_read = read(fifo_pid, &buf, 1024)) > 0) {
        write(1,&buf,bytes_read);
    }



    


    
    unlink(pid);
    close(fifo_fd);
    close(fifo_pid);

    return 0;
}