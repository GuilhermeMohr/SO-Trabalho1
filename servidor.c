#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "thread.h"

pthread_mutex_t mutex;
int receiveFile;

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

void EnviarRespostas(char *input) {
    int n = strlen(input)+1;
    if (write(receiveFile, &n, sizeof(int)) == -1) { exit(1); }

    if (write(receiveFile, input, n) == -1) { exit(2); }
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
    char var1[15] = "";
    char var2[15] = "";

    strncpy(input, string, sizeof(input) - 1);

    strtok(input, " ");
    char *token = strtok(NULL, " ");
    if (token != NULL) {
        strncpy(var1, token, sizeof(var1) - 1);
    } else {
        EnviarRespostas("Comando não é valido!\n");
        return NULL;
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strncpy(var2, token, sizeof(var2) - 1);
    }

    pthread_mutex_lock(&mutex);
    FILE *read = fopen("banco.txt", "r");
    if (!read) {
        EnviarRespostas("Erro ao abrir banco.txt\n");
        return NULL;
    }

    char linhas[40] = "";
    char linha[20] = "";
    
    while (fgets(linha, sizeof(linha), read)) {
        if ((strstr(linha, var1) != NULL && strstr(var1, "id") != NULL) || (var2[0] != '\0' && strstr(var2, "id") != NULL && strstr(linha, var2) != NULL)) {
            fclose(read);
            EnviarRespostas("Id já existe no banco de dados!\n");
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
    }
    fclose(read);
    pthread_mutex_unlock(&mutex);
    
    char text[30] = "{\n";
    strcat(text, var1);

    if ((strstr(var1, "id") == NULL) &&
        (var2[0] == '\0' || strstr(var2, "id") == NULL)) {
        EnviarRespostas("Id precisa ser informado!\n");
        return NULL;
    }

    if (!CheckValues(var1)) {
        return NULL;
    }

    if (var2[0] != '\0') {
        strcat(text, "\n");
        strcat(text, var2);

        if (!CheckValues(var2)) {
            return NULL;
        }
    }

    strcat(text, "\n}\n");

    pthread_mutex_lock(&mutex);
    FILE *banco = fopen("banco.txt", "a");
    fprintf(banco, "%s", text);

    EnviarRespostas("Informação adicionada com sucesso!\n");

    fclose(banco);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *Selecionar(void* string) {
    char input[200];
    strncpy(input, string, sizeof(input) - 1);
    input[sizeof(input) - 1] = '\0';

    char camposSelecionados[10][50];
    int totalCampos = 0;

    char campoCondicao[50] = "";
    char valorCondicao[50] = "";

    char *token = strtok(input, " ");
    if (token == NULL || strcasecmp(token, "select") != 0) {
        EnviarRespostas("Comando inválido! (esperado: select <campos> where <campo>=<valor>)\n");
        return NULL;
    }

    while ((token = strtok(NULL, " ")) != NULL) {
        if (strcasecmp(token, "where") == 0) {
            break;
        }
        if (totalCampos < 10) {
            strncpy(camposSelecionados[totalCampos++], token, 49);
        }
    }

    if (totalCampos == 0 || token == NULL) {
        EnviarRespostas("Nenhum campo selecionado ou cláusula WHERE ausente!\n");
        return NULL;
    }

    token = strtok(NULL, "=");
    if (token == NULL) {
        EnviarRespostas("Campo de condição ausente!\n");
        return NULL;
    }
    strncpy(campoCondicao, token, sizeof(campoCondicao) - 1);

    token = strtok(NULL, "");
    if (token == NULL) {
        EnviarRespostas("Valor de condição ausente!\n");
        return NULL;
    }
    strncpy(valorCondicao, token, sizeof(valorCondicao) - 1);

    pthread_mutex_lock(&mutex);
    FILE *banco = fopen("banco.txt", "r");
    if (!banco) {
        EnviarRespostas("Erro ao abrir banco.txt\n");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    char linhas[160] = "";
    char linha[80] = "";
    int encontrado = 0;

    char condBusca[100];
    snprintf(condBusca, sizeof(condBusca), "%s=%s", campoCondicao, valorCondicao);

    char resposta[200] = "";
    while (fgets(linha, sizeof(linha), banco)) {
        strcat(linhas, linha);

        if (strchr(linha, '}') != NULL) {
            if (strstr(linhas, condBusca)) {
                strcat(resposta, "Resultado: ");
                for (int i = 0; i < totalCampos; i++) {
                    char *inicio = strstr(linhas, camposSelecionados[i]);
                    if (inicio) {
                        char *valor = strchr(inicio, '=');
                        if (valor) {
                            valor++;
                            char resultado[100] = "";
                            int j = 0;
                            while (*valor && *valor != '\n' && *valor != ',' && *valor != '}') {
                                resultado[j++] = *valor++;
                            }
                            resultado[j] = '\0';
                            strcat(resposta, camposSelecionados[i]);
                            strcat(resposta, "=");
                            strcat(resposta, resultado);
                            strcat(resposta, " ");
                        }
                    } else {
                        strcat(resposta, camposSelecionados[i]);
                        strcat(resposta, "=(não encontrado) ");
                    }
                }
                strcat(resposta, "\n");
                encontrado = 1;
            }
            memset(linhas, 0, sizeof(linhas));
        }
    }

    fclose(banco);
    pthread_mutex_unlock(&mutex);

    if (!encontrado) {
        EnviarRespostas("Nenhum resultado encontrado com a condição fornecida.\n");
    } else {
        EnviarRespostas(resposta);
    }

    return NULL;
}

void *Atualizar(void* string) {
    char input[200];
    strncpy(input, string, sizeof(input) - 1);

    char var1[15] = "", var2[15] = "";

    strtok(input, " ");
    char *token = strtok(NULL, " ");
    if (token != NULL) {
        strncpy(var1, token, sizeof(var1) - 1);
        if(!is_number(var1)) {
            EnviarRespostas("Comando não é válido!\n");
            return NULL;
        }
    } else {
        EnviarRespostas("Comando não é válido!\n");
        return NULL;
    }

    token = strtok(NULL, " ");
    if (token == NULL || strcmp(token, "values")) {
        EnviarRespostas("Comando não é válido!\n");
        return NULL;
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strncpy(var2, token, sizeof(var2) - 1);
    } else {
        EnviarRespostas("Comando não é válido!\n");
        return NULL;
    }

    if (strstr(var2, "nome=") == NULL) {
        EnviarRespostas("Comando não é válido!\n");
        return NULL;
    }

    pthread_mutex_lock(&mutex);

    FILE *original = fopen("banco.txt", "r");
    if (!original) {
        EnviarRespostas("Erro ao abrir banco.txt\n");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    FILE *temporario = fopen("temp.txt", "w");
    if (!temporario) {
        fclose(original);
        EnviarRespostas("Erro ao criar temp.txt\n");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    char linhas[160];
    char linha[40];
    int encontrado = 0;

    char id[20] = "";
    snprintf(id, sizeof(id), "id=%s", var1);
    while (fgets(linha, sizeof(linha), original)) {
        strcat(linhas, linha);

        if (strstr(linha, "}") != NULL) {
            if (strstr(linhas, id) != NULL){
                char *nome_pos = strstr(linhas, "nome=");
                if (nome_pos) {
                    char *value_start = nome_pos + strlen("nome=");
                    char *value_end = strchr(value_start, '\n');

                    char updated_text[512];
                    int prefix_len = value_start - linhas;

                    strncpy(updated_text, linhas, prefix_len);
                    updated_text[prefix_len] = '\0';

                    strtok(var2, "=");
                    strcat(updated_text, strtok(NULL, "="));
                    if (value_end) strcat(updated_text, value_end);

                    strcpy(linhas, updated_text);
                }

                encontrado = 1;
            }
            fputs(linhas, temporario);
            memset(linhas, 0, sizeof(linhas));
        } 
    }

    fclose(original);
    fclose(temporario);

    remove("banco.txt");
    rename("temp.txt", "banco.txt");

    pthread_mutex_unlock(&mutex);

    if (encontrado) {
        EnviarRespostas("Informação atualizada com sucesso!\n");
    } else {
        EnviarRespostas("Id não encontrado para atualizar!\n");
    }

    return NULL;
}

void *Deletar(void* string) {
    int encontrado = 0;

    char input[200] = "";
    char var1[7] = "";

    strncpy(input, string, sizeof(input) - 1);

    strtok(input, " ");
    if ((string = strtok(NULL, " ")) != NULL) {
        strcpy(var1, string);
    } else {
        EnviarRespostas("Comando não é valido!\n");
    }

    if (!is_number(var1)) {
        EnviarRespostas("Comando não é valido!\n");
        return NULL;
    }

    pthread_mutex_lock(&mutex);
    FILE *original = fopen("banco.txt", "r");
    if (!original) {
        EnviarRespostas("Erro ao abrir banco.txt\n");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    
    FILE *temporario = fopen("temp.txt", "w");
    if (!temporario) {
        fclose(original);
        EnviarRespostas("Erro ao criar temp.txt\n");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    char linhas[160] = "";
    char linha[40] = "";
    char idbusca[20] = "";
    snprintf(idbusca, sizeof(idbusca), "id=%s", var1);

    while (fgets(linha, sizeof(linha), original)) {
        strcat(linhas, linha);

        if (strstr(linha, "}") != NULL) {
            if (strstr(linhas, idbusca) == NULL) {
                fputs(linhas, temporario);
            } else {
                encontrado = 1;
            }
            memset(linhas, 0, sizeof(linhas));
        }
    }

    remove("banco.txt");
    rename("temp.txt", "banco.txt");
    
    fclose(temporario);
    fclose(original);
    pthread_mutex_unlock(&mutex);

    if (encontrado == 0) {
        EnviarRespostas("Arquivo não encontrado!\n");
    } else {
        EnviarRespostas("Informação deletada com sucesso!\n");
    }

    return NULL;
}

int main()
{
    pthread_mutex_init(&mutex, NULL);

    Thread* thread_inicial = NULL;
    Thread* thread_final = NULL;
    char input[200];
    Fifo("sendfifo");
    Fifo("receivefifo");

    printf("Abrindo conexão pipe\n");
    int sendFile = open("sendfifo", O_RDONLY);
    receiveFile = open("receivefifo", O_WRONLY);

    int n = 0;
    do {
        if (read(sendFile, &n, sizeof(int)) == -1) { return 1; }
    
        if (read(sendFile, input, n) == -1) { return 2; }
        if (!strcmp(input, "exit")) { break; }

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
            EnviarRespostas("Comando não é valido!\n");
        }
    } while(1);

    FinalizarThreads(thread_inicial, thread_final);

    close(sendFile);
    close(receiveFile);

    pthread_mutex_destroy(&mutex);

    return 0;
}
