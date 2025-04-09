#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

int main()
{
    int pipeDescriptor[2];
    // 0 - Read
    // 1 - Write

    if (pipe(pipeDescriptor) == -1) {
        printf("Erro ao abrir pipe!");
        return 1;
    };

    char key_code = 'a';
    do {
        if (_kbhit()) {
            key_code = _getch();
        }

    } while(key_code != 0x1B);

    return 0;
}
