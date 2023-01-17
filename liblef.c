#include <stdio.h>
#include <stdlib.h>
#include "liblef.h"
#include <string.h>

/* 
 * Cria uma LEF vazia 
 */
lef_t *cria_lef (){
    lef_t *lef = malloc(sizeof(lef_t));
    if (!lef)
        return NULL;

    lef->Primeiro = NULL;

    return lef;
}

/* Destroi os nos de uma lef. */
void destroi_no_lef(nodo_lef_t *no){
    if (no->prox != NULL){
        destroi_no_lef(no->prox);
    }
    free(no->evento);
    free(no);
}

/* 
 * Destroi a LEF e retorna NULL. 
 */ 
lef_t *destroi_lef (lef_t *l){
    if (!l)
        return NULL;

    if (l->Primeiro != NULL){
        destroi_no_lef (l->Primeiro);
    }
    free(l); 

    return NULL;
}

/* 
 * Adiciona um evento na primeira posicao da LEF. Uma nova copia
 * da estrutura evento sera feita. 
 * Retorna 1 em caso de sucesso, 0 caso contrario.
 */ 
int adiciona_inicio_lef (lef_t *l, evento_t *evento){
    evento_t *novo_eve = malloc(sizeof(evento_t));
    if(!novo_eve)
        return 0;

    nodo_lef_t *no = malloc(sizeof(nodo_lef_t));
    if (!no){
        free(novo_eve);
        return 0;
    }

    no->evento = novo_eve;
    memcpy(novo_eve, evento, sizeof(evento_t));
    if (!l->Primeiro){
        l->Primeiro = no;
        no->prox = NULL;
        return 1;
    }
    no->prox = l->Primeiro;
    l->Primeiro = no;

    return 1;
}

/* Posiciona no tempo certo o nodo_lef_t. */
void posiciona_ordem(lef_t *l, nodo_lef_t *no){
    nodo_lef_t *aux = l->Primeiro;

    if (aux->evento->tempo > no->evento->tempo){
        no->prox = l->Primeiro;
        l->Primeiro = no;
        return;
    }
    while (aux->prox != NULL && aux->prox->evento->tempo < no->evento->tempo){
        aux = aux->prox;
    }
    if (aux->prox == NULL){
        aux->prox = no;
        no->prox = NULL;
        return;
    }
    no->prox = aux->prox;
    aux->prox = no;
}
/* 
 * Adiciona um evento de acordo com o valor evento->tempo na LEF. 
 * Uma nova copia da estrutura evento sera feita 
 * Retorna 1 em caso de sucesso, 0 caso contrario.
 */
int adiciona_ordem_lef (lef_t *l, evento_t *evento){
    evento_t *novo_eve = malloc(sizeof(evento_t));
    if(!novo_eve)
        return 0;

    nodo_lef_t *no = malloc(sizeof(nodo_lef_t));
    if (!no){
        free(novo_eve);
        return 0;
    }


    no->evento = novo_eve;
    memcpy(novo_eve, evento, sizeof(evento_t));
    if (!l->Primeiro){
        l->Primeiro = no;
        no->prox = NULL;
        return 1;
    }
    posiciona_ordem(l, no);

    return 1;
}
/* 
 * Retorna e retira o primeiro evento da LEF. 
 * A responsabilidade por desalocar
 * a memoria associada eh de quem chama essa funcao.
 */
evento_t *obtem_primeiro_lef (lef_t *l){
    if (l->Primeiro == NULL)
        return NULL;

    evento_t *aux = l->Primeiro->evento;
    nodo_lef_t *_no = l->Primeiro;
    l->Primeiro = _no->prox;
    free(_no);

    return aux;
}
