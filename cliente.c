#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "banco.h"
//#include <pthread.h>

int Fifo(char name[]) {
    if (mkfifo(name, 0777) == -1) {
        if(errno != EEXIST) {
            printf("Erro ao abrir fifo!\n");
            exit(1);
        }
    }
}

int main()
{
    char input[100];
    Fifo("sendfifo");
    Fifo("receivefifo");

    printf("Abrindo conexão pipe");
    int sendFile = open("sendfifo", O_WRONLY);

    do {
        printf("Insira o comando que deseja executar!\n");
        scanf("%s", input);
        
        if (write(sendFile, input, sizeof(input)) == -1) { return 2; }
    } while(strcmp(input,  "exit"));

    close(fd);

    return 0;
}
