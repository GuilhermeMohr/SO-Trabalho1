#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "banco.h"
#include "thread.h"

int Fifo(char name[]) {
    if (mkfifo(name, 0777) == -1) {
        if(errno != EEXIST) {
            printf("Erro ao abrir fifo!\n");
            exit(3);
        }
    }
}

int is_number(const char *str) {
    if (*str == '\0') return 0;  // string vazia não é número
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

int CheckValues(char string[]) {
    char *chave = strtok(string, "=");
    char *valor = strtok(NULL, "=");
    if (strcmp(chave, "id") == 0) {
        if (!is_number(valor)) {
            printf("Id deveria ser um valor numérico!\n");
            return 0;
        }
    } else if (strcmp(chave, "nome") != 0) {
        printf("Comando inválido parametros errados!\n");
        return 0;
    }
    return 1;
}

void *Inserir(void* string) {
    char input[200];
    char var1[15];
    char var2[15];

    strncpy(input, string, sizeof(input) - 1);

    strtok(input, " ");
    if ((string = strtok(NULL, " ")) != NULL) strcpy(var1, string);
    if ((string = strtok(NULL, " ")) != NULL) strcpy(var2, string);

    if (var1 == NULL) {
        printf("Comando não é valido!\n");
        return NULL;
    }

    FILE *read = fopen("banco.txt", "r");

    char linhas[40];
    char linha[20];
    
    while (fgets(linha, sizeof(linha), read)) {
        if ((strstr(linha, var1) != NULL && strstr(var1, "id") != NULL) || (var2 != NULL && strstr(var2, "id") != NULL && strstr(linha, var2) != NULL)) {
            fclose(read);
            printf("Id já existe no banco de dados!\n");
            return NULL;
        }
    }
    fclose(read);
    
    char text[30] = "{\n";
    strcat(text, var1);

    if (!CheckValues(var1)) {
        return NULL;
    }

    if (var2 != NULL) {
        strcat(text, "\n");
        strcat(text, var2);

        if (!CheckValues(var2)) {
            return NULL;
        }
    }
    strcat(text, "\n}\n");

    FILE *banco = fopen("banco.txt", "a");
    fprintf(banco, text);

    printf("Informação adicionada com sucesso!\n");

    fclose(banco);
    return NULL;
}

void *Selecionar(void* string) {
    FILE *banco = fopen("banco.txt", "a");
    fprintf(banco, "ok");

    fclose(banco);
    return NULL;
}

void *Atualizar(void* string) {
    FILE *banco = fopen("banco.txt", "a");
    fprintf(banco, "ok");

    fclose(banco);
    return NULL;
}

void *Deletar(void* string) {
    int encontrado = 0;

    char input[200];
    char var1[7];

    strncpy(input, string, sizeof(input) - 1);

    strtok(input, " ");
    if ((string = strtok(NULL, " ")) != NULL) strcpy(var1, string);

    if (var1 == NULL || !is_number(var1)) {
        printf("Comando não é valido!\n");
        return NULL;
    }

    FILE *original = fopen("banco.txt", "r");
    FILE *temporario = fopen("temp.txt", "w");

    char linhas[80];
    char linha[20];
    char idbusca[20];
    snprintf(idbusca, sizeof(idbusca), "id=%s", var1);

    while (fgets(linha, sizeof(linha), original)) {
        strcat(linhas, linha);

        if (strstr(linha, "}") != NULL && strstr(linhas, idbusca) == NULL) {
            fputs(linhas, temporario);
            memset(linhas, 0, sizeof(linhas));
        } else {
            encontrado = 1;
        }
    }

    remove("banco.txt");
    rename("temp.txt", "banco.txt");
    
    fclose(temporario);
    fclose(original);

    if (encontrado == 0) {
        printf("Arquivo não encontrado!\n");
    } else {
        printf("Informação deletada com sucesso!\n");
    }

    return NULL;
}

int main()
{
    Thread* thread_inicial = NULL;
    Thread* thread_final = NULL;
    char input[200];
    Fifo("sendfifo");
    Fifo("receivefifo");

    printf("Abrindo conexão pipe\n");
    int sendFile = open("sendfifo", O_RDONLY);
    int receiveFile = open("receivefifo", O_WRONLY);

    int n = 0;
    do {
        if (read(sendFile, &n, sizeof(int)) == -1) { return 1; }
    
        if (read(sendFile, input, n) == -1) { return 2; }
        if (!strcmp(input, "exit")) { break; }

        for (int i=0; i<strlen(input); i++) {
            input[i] = tolower(input[i]);
        }

        if (strncmp(input, "select", 6) == 0) {
            pthread_t thread;
            AdicionarThread(&thread_inicial, &thread_final, thread);
            pthread_create(&thread_final->thread, NULL, Selecionar, (void*) input);
        } else if (strncmp(input, "insert", 6) == 0) {
            pthread_t thread;
            AdicionarThread(&thread_inicial, &thread_final, thread);
            pthread_create(&thread_final->thread, NULL, Inserir, (void*) input);
        } else if (strncmp(input, "delete", 6) == 0) {
            pthread_t thread;
            AdicionarThread(&thread_inicial, &thread_final, thread);
            pthread_create(&thread_final->thread, NULL, Deletar, (void*) input);
        } else if (strncmp(input, "update", 6) == 0) {
            pthread_t thread;
            AdicionarThread(&thread_inicial, &thread_final, thread);
            pthread_create(&thread_final->thread, NULL, Atualizar, (void*) input);
        } else {
            printf("Comando não é valido!\n");
        }
    } while(1);

    FinalizarThreads(thread_inicial, thread_final);

    close(sendFile);
    close(receiveFile);

    return 0;
}
