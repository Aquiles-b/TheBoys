#include <stdio.h>
#include <stdlib.h>
#include "libfila.h"

/* Cria uma fila vazia e a retorna, se falhar retorna NULL. */
fila_t *cria_fila (){
    fila_t *nova = malloc(sizeof(fila_t));
    if(!nova)
        return NULL;
    nova->tamanho = 0;
    nova->fim = NULL;
    nova->ini = NULL;
    
    return nova;
}

/* Remove os no da fila recursivamente. */
void destroi_no_fila (nodo_f_t *no){
    if(!no)
        return;
 
    destroi_no_fila(no->prox);
    free(no);
}

/* Remove todos os elementos da fila, libera espaco e devolve NULL. */
fila_t *destroi_fila (fila_t *f){
    if(!f)
        return NULL;
    
    destroi_no_fila(f->ini);
    free(f);
    
    return NULL;
}

/* Retorna 1 se a fila esta vazia e 0 caso contrario. */
int vazia_fila (fila_t *f){
    if (f->tamanho > 0)
        return 0;
    
    return 1;
}

/* Retorna o numero de elementos da fila. */
int tamanho_fila (fila_t *f){
    
    return f->tamanho;
}

/* Insere o elemento no final da fila (politica FIFO).
 * Retorna 1 se a operacao foi bem sucedida e 0 caso contrario. */
int insere_fila (fila_t *f, int elemento){
    nodo_f_t *no = malloc(sizeof(nodo_f_t));
    if(!no)
        return 0;

    no->elem = elemento;
    no->prox = NULL;
    if(vazia_fila(f)){
        f->ini = no;
        f->fim = no;
    }else{
        (f->fim)->prox = no;
        f->fim = no;
    }
    f->tamanho++;

    return 1;
}

/* Remove o elemento do inicio da fila (politica FIFO) e o retorna
 * no parametro *elemento. Retorna 1 se a operacao foi bem sucedida
 * e 0 caso contrario. */
int retira_fila (fila_t *f, int *elemento){
    nodo_f_t *aux;
    if(vazia_fila(f))
        return 0;
    
    aux = f->ini;
    f->ini = aux->prox;
    *elemento = aux->elem;
    f->tamanho--;
    free(aux);
    
    return 1;
}

/* Auxilia a impressao dos elementos da fila. */
void imprime_no_fila(nodo_f_t *no){
    if(!no)
        return;

    printf("%d ", no->elem);
    imprime_no_fila(no->prox);
}

/* para depuracao */
void imprime_fila (fila_t *f){
    if(!f->ini)
        return;
    
    imprime_no_fila(f->ini);
    printf("\n");
}

