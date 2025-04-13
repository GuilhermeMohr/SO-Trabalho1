#include <pthread.h>

typedef struct ThreadNode {
    pthread_t thread;
    struct ThreadNode* prox;
} Thread;

void AdicionarThread(Thread** inicio, Thread** fim, pthread_t novaThread) {
    Thread* nova = (Thread*) malloc(sizeof(Thread));
    if (nova == NULL) return;

    nova->thread = novaThread;
    nova->prox = NULL;

    if (*inicio == NULL) {
        *inicio = nova;
        *fim = nova;
    } else {
        (*fim)->prox = nova;
        *fim = nova;
    }

    printf("Thread criada!\n");
}

void FinalizarThreads(Thread* inicio, Thread* fim) {
    if (inicio == NULL) {
        return;
    }
    Thread *aux = inicio;
    while(aux->prox != NULL){
        pthread_join(aux->thread, NULL);
        Thread *aux1 = aux->prox;
        free(aux);
        aux = aux1;
    }
    free(aux);
}