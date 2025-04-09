#include <stdio.h>
#include <stdlib.h>
//#include <pthread.h>

int main()
{
    char input[100];
    printf("Insira o comando que deseja executar!\n");
    scanf("%s", input);
    printf("%s", input);

    char key_code = 'a';

    do {
        if (_kbhit()) {
            key_code = _getch();
        }

    } while(key_code != 0x1B);

    return 0;
}
