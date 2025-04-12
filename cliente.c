#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

int Fifo(char name[]) {
    if (mkfifo(name, 0777) == -1) {
        if(errno != EEXIST) {
            printf("Erro ao abrir fifo!\n");
            exit(3);
        }
    }
}

int main()
{
    char input[200];
    Fifo("sendfifo");
    Fifo("receivefifo");

    printf("Abrindo conexão pipe\n");
    int sendFile = open("sendfifo", O_WRONLY);
    int receiveFile = open("receivefifo", O_RDONLY);

    do {
        printf("Insira o comando que deseja executar!\n");
        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = 0;
        
        int n = strlen(input)+1;
        if (write(sendFile, &n, sizeof(int)) == -1) { return 1; }

        if (write(sendFile, input, n) == -1) { return 2; }
    } while(strcmp(input,  "exit"));

    close(sendFile);
    close(receiveFile);

    return 0;
}
