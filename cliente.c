#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "banco.h"
//#include <pthread.h>

int main()
{
    char input[100];
    mkfifo("./fifos/clientefifo", 0777);
    int fd = open("./fifos/clientefifo", O_WRONLY);

    do {
        printf("Insira o comando que deseja executar!\n");
        scanf("%s", input);
        printf("%s\n", input);

    } while(strcmp(input,  "exit"));

    close(fd);

    return 0;
}
