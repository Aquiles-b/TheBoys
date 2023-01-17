#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libconjunto.h"

conjunto_t *cria_cjt (int max) {
    conjunto_t	*c = malloc(sizeof(conjunto_t));
    if (c == NULL)
        return NULL;

    c->v = malloc(sizeof(int)*max);
    c->max = max;
    c->card = 0;
    c->ptr = 0;

    return c;
}

conjunto_t *destroi_cjt (conjunto_t *c) {
    free(c->v);
    free(c);

    return NULL;
}

int vazio_cjt (conjunto_t *c) {

    return c->card == 0;
}

int cardinalidade_cjt (conjunto_t *c) {

    return c->card;
}

int insere_cjt (conjunto_t *c, int elemento) {
    if (c->card == c->max)
        return 0;

    if ( !pertence_cjt(c, elemento) ) {
        c->v[c->card] = elemento;
        c->card++;
    }

    return 1;
}

int retira_cjt (conjunto_t *c, int elemento) {
    int i;
    if (!pertence_cjt(c, elemento))
        return 0;

    for (i = 0; i < cardinalidade_cjt(c); i++) {
        if (c->v[i] == elemento) {
            c->v[i] = c->v[cardinalidade_cjt(c)-1];
            (c->card)--;
            return 1;
        }
    }

    return 0;
}

int pertence_cjt (conjunto_t *c, int elemento) {
    inicia_iterador_cjt(c);
    int elemento_cjt;

    while (incrementa_iterador_cjt(c, &elemento_cjt)) {
        if (elemento_cjt == elemento)
            return 1;
    }

    return 0;
}

int contido_cjt (conjunto_t *c1, conjunto_t *c2) {
    int i = 0;

    while (i < c1->card) {
        if ( !pertence_cjt(c2, c1->v[i]) )
            return 0;
        i++;
    }

    return 1;
}

int sao_iguais_cjt (conjunto_t *c1, conjunto_t *c2) {
    if (c1->card == c2->card){

        if (contido_cjt(c1, c2)) /* Um conjunto está contido nele mesmo*/
            return 1;
    }
    return 0;
}

conjunto_t *diferenca_cjt (conjunto_t *c1, conjunto_t *c2) {
    int i;
    conjunto_t *cjt_diff = cria_cjt(c1->max);

    if (!cjt_diff)
        return NULL;

    for (i = 0; c1->card > i; i++){
        if ( !pertence_cjt(c2, c1->v[i]) )
            insere_cjt(cjt_diff, c1->v[i]);
    }

    return cjt_diff;
}

conjunto_t *interseccao_cjt (conjunto_t *c1, conjunto_t *c2) {
    int i;
    conjunto_t *cjt_inter = cria_cjt(c1->max);

    if (!cjt_inter)
        return NULL;

    for (i = 0; c1->card > i; i++){
        if ( pertence_cjt(c2, c1->v[i]) )
            insere_cjt(cjt_inter, c1->v[i]);
    }

    return cjt_inter;
}

/*Ajuda a funcao uniao_cjt a juntar os conjuntos.*/
int aux_uniao (conjunto_t *c, conjunto_t *uni) {
    int i;
    for (i = 0; i < c->card; i++) {
        if ( !insere_cjt(uni, c->v[i]) )
            return 0;
    }
    return 1;
}

conjunto_t *uniao_cjt (conjunto_t *c1, conjunto_t *c2) {
    conjunto_t * cjt_uni = cria_cjt(c1->max);
    if (!cjt_uni)
        return NULL;
    aux_uniao(c1, cjt_uni);

    if(!aux_uniao(c2, cjt_uni)){  /* se nao for possivel juntar o segundo, retorna NULL.*/
        destroi_cjt(cjt_uni);
        return NULL;
    }

    return cjt_uni;
}

conjunto_t *copia_cjt (conjunto_t *c) {
    conjunto_t * cp_cjt = cria_cjt(c->max);
    if (!cp_cjt)
        return NULL;

    memcpy(cp_cjt->v, c->v, sizeof(int)*c->card);
    cp_cjt->card = c->card;
    cp_cjt->ptr = c->ptr;

    return cp_cjt;
}

conjunto_t *cria_subcjt_cjt (conjunto_t *c, int n) {
    if (n >= c->card)
        return copia_cjt(c);

    int i = 0;
    int indice;
    conjunto_t *sub_cjt = cria_cjt(c->max);
    if (!sub_cjt)
        return NULL;

    while (i < n) {
        indice = rand()% c->card;
        if ( !pertence_cjt(sub_cjt, c->v[indice]) ){
            insere_cjt(sub_cjt, c->v[indice]);
            i++;
        }
    }

    return sub_cjt;
}

/*Ordena um vetor de ponteiros que apontam para elementos
 * do conjunto que vai ser imprimido.*/
int **ordena (int *vet, int tam){
    int **ordenado = malloc(sizeof(int*)*tam);
    int menor, i, j;
    int *aux;
    for (i = 0; i < tam; i++) {
        ordenado[i] = &vet[i];
    }
    for (i = 0; i < tam-1; i++) {
        menor = i;
        for (j = i+1; j < tam; j++) {
            if (*ordenado[menor] > *ordenado[j])
                menor = j;
        }
        aux = ordenado[i];
        ordenado[i] = ordenado[menor];
        ordenado[menor] = aux;
    }

    return ordenado;
}

void imprime_cjt (conjunto_t *c) {
    if (vazio_cjt(c)) {
        printf ("conjunto vazio\n");
        return;
    }
    int **vet_ordenado = ordena (c->v, c->card);
    int i;
    for (i = 0; i < c->card; i++) {
        printf ("%d", *vet_ordenado[i]);
        if (i != c->card-1)
            printf (" ");
        else
            printf ("\n");
    }

    free(vet_ordenado);
}

void inicia_iterador_cjt (conjunto_t *c) {
    c->ptr = 0;
}

int incrementa_iterador_cjt (conjunto_t *c, int *ret_iterador) {
    if (c->ptr == c->card)
        return 0;

    *ret_iterador = c->v[c->ptr];
    (c->ptr)++;

    return 1;
}

int retira_um_elemento_cjt (conjunto_t *c) {
    int elemento, indice;

    indice = rand() % c->card;
    elemento = c->v[indice];
    retira_cjt (c, elemento);

    return elemento;
}
