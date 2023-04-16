#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_BUFFER_SIZE 1024


void ctrl_c(int signum){
    // falta completar isto mas com ajuda
}


int main(int argc, char* argv[]){
    
    int fifo_fd, bytes_read;
    int fifo_pid;
    char buf[MAX_BUFFER_SIZE];


    // Cria o fifoCS
    if(mkfifo("fifoCS",0666) == -1){ 
        perror("ERRO AO CRIAR FIFOCS");
        unlink("fifoCS");
        return -1;
    }
    
    // Utiliza o fifoCS para ler
    if ((fifo_fd = open("fifoCS",O_RDONLY)) == -1) {  // abre o fifo para ler
       perror("Open");
   }    else {
       printf("Opened FIFO for reading\n");
   }

   

    // inicilizar a lista ligada fora do while

    // while 1 para rodar sempre e o read é por causa de ser bloqueador
    while ((1))
    {
    
        while((bytes_read =read(fifo_fd, &buf, MAX_BUFFER_SIZE)) > 0) {
            
            char linha[bytes_read]; // linha que o cliente escreve com o PID
            
            strcpy(linha,buf);
            write(1, &linha, strlen(linha)); // testar para ver se copia para a string 
            
            // fazer as merdas todas

            char* token = strtok(linha," "); // nome do FIFO do cliente
            char* primeiro_token = token;
            token = strtok(NULL," ");

            
            if ((fifo_pid = open(primeiro_token,O_WRONLY)) == -1)
                perror("Open");

            write(fifo_pid,primeiro_token,strlen(primeiro_token));
            write(fifo_pid,token,strlen(primeiro_token));
            
            //char* resultado = "O resultado do seu pedido foi NADA\n";  // teste para ver se funciona
            
            //write(fifo_pid, resultado,strlen(resultado)); 
            
            close(fifo_pid);

            memset(buf,0,MAX_BUFFER_SIZE); // limpa o buffer do fifo
            
            //signal(SIGQUIT,ctrl_c); nao sei onde por isto mas o suposto é dar close no fifo e unlink e terminar o programa quando fazer ctrl+c
        } 
    }

    unlink("fifoCS");
    close(fifo_fd);
    
    return 0;
}