#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liblef.h"
#include "libfila.h"
#include "libconjunto.h"

#define TIPO_CHEGADA  1
#define TIPO_SAIDA  2
#define TIPO_MISSAO  3
#define TIPO_FIM  4
#define FIM_DO_MUNDO 34944
#define N_HABS 10
#define N_HEROIS N_HABS*5  
#define N_LOCAIS N_HEROIS/6 
#define INICIO 1
#define ORDEM 0
#define MAX_HABS 5
#define MAX_LOT 30

typedef struct coord{
    int x;
    int y;
} coord_t;

typedef struct mensagem{
    int tipo;
    int dado;
    conjunto_t **missao;
} mensagem_t;

typedef struct heroi{
    int id;
    int paciencia;
    int idade;
    int xp;
    conjunto_t *habs;
} heroi_t;

typedef struct local{
    int id;
    int lot_max;
    conjunto_t *equipe;
    fila_t *fila;
    coord_t coord;
} local_t;

typedef struct mundo{
    int tempo_atual;
    heroi_t **herois;
    local_t **locais;
    conjunto_t *todas_habs;
    int num_herois;
    int num_locais;
    int tam_mundo;
} mundo_t;

int aleat(int a, int b){

    return rand() % (b + 1 - a) + a;
}

/*Devolve o tempo de permanencia de um heroi num local sendo no minimo 1.*/
int gera_tempo_permanencia(int paciencia){
    int tpl = paciencia/10 + aleat(-2, 6);

    if (tpl < 1)
        return 1;

    return tpl;
}

/*Calcula distancia entre dois pontos cartesianos em R².*/
int distancia_pontos(coord_t a, coord_t b){
    coord_t vet;
    double distancia;

    vet.x = b.x - a.x;
    vet.y = b.y - a.y;
    distancia = sqrt(pow(vet.x, 2) + pow(vet.y, 2));

    return ceil(distancia);
}

/*Devolve o tempo de deslocamento que um heroi leva de um local a outro.*/
int gera_tempo_deslocamento(heroi_t *heroi, local_t *local_a, local_t *local_b){
    int velocidade, distancia;

    distancia = distancia_pontos(local_a->coord,local_b->coord); 
    velocidade = heroi->idade - 40;
    if (velocidade < 0)
        velocidade = 0;
    velocidade = 100 - velocidade;

    return distancia/velocidade;
}

/*Cria um evento do tipo saida e o insere na lef.*/
void insere_evento_saida(int id_heroi, int id_local, int tempo, lef_t *l){
    evento_t eve_saida;

    eve_saida.tipo = TIPO_SAIDA;
    eve_saida.dado1 = id_heroi;
    eve_saida.dado2 = id_local;
    eve_saida.tempo = tempo;
    adiciona_ordem_lef(l, &eve_saida);
}

/*Cria evento de chegada e o insere no inicio ou no fim da lef dependendo
 * do paramentro @posi.*/
void insere_evento_chegada(int id_heroi, int id_local, int tempo, lef_t *l, int posi){
    evento_t eve_chegada;

    eve_chegada.tipo = TIPO_CHEGADA;
    eve_chegada.dado1 = id_heroi;
    eve_chegada.dado2 = id_local;
    eve_chegada.tempo = tempo;

    if (posi)
        adiciona_inicio_lef(l, &eve_chegada);
    else
        adiciona_ordem_lef(l, &eve_chegada);
}

/*Cria um vetor de conjuntos a partir do indice 1 
 * com todas as habilidades de cada equipe
 * em cada local.*/
void habilidades_das_equipes (mundo_t *mundo, mensagem_t *msg_missao) {
    int i, j;
    heroi_t *heroi_da_equipe;
    conjunto_t *aux_desaloca; /*uniao_cjt aloca sempre um novo conjuto, entao eh necessario para desalocar o antigo.*/

    for (i = 1; i <= mundo->num_locais; i++) {
        msg_missao->missao[i] = cria_cjt(MAX_HABS*MAX_LOT);
        for (j = 0; j < cardinalidade_cjt(mundo->locais[i-1]->equipe); j++) {
            aux_desaloca = msg_missao->missao[i];
            /*Coloca as habilidades de um heroi da equipe no conjunto de todas as habilidades da equipe.*/
            heroi_da_equipe = mundo->herois[mundo->locais[i-1]->equipe->v[j]];
            msg_missao->missao[i] = uniao_cjt(msg_missao->missao[i], heroi_da_equipe->habs);
            aux_desaloca = destroi_cjt (aux_desaloca); /*Desaloca uniao antiga.*/
        }
    }
}

/*Acha a menor equipe para fazer a missao e coloca o indice na variavel dado.*/
void menor_equipe_apta(mensagem_t *msg){
    int i;
    msg->dado = -1;
    for (i = 1; i <= N_LOCAIS; i++){
        if (contido_cjt(msg->missao[0], msg->missao[i])){
            if (msg->dado == -1){
                msg->dado = i;
            }
            else if (cardinalidade_cjt(msg->missao[i]) < cardinalidade_cjt(msg->missao[msg->dado])){
                msg->dado = i;
            }
        }
    }
}

/*Acha a menor equipe para realizar a missao. Os dados sao guardados 
 * na struct mensagem_t.*/
void equipe_escolhida(mundo_t *mundo, mensagem_t *msg_missao){
    habilidades_das_equipes(mundo, msg_missao);
    menor_equipe_apta(msg_missao);
}

/*Aumenta em 1 o xp de todos os herois do local.*/
void aumenta_xp_equipe(mundo_t *mundo, int equipe){
    int i;
    local_t *local = mundo->locais[equipe];

    for (i = 0; i < cardinalidade_cjt(local->equipe); i++){
        mundo->herois[local->equipe->v[i]]->xp++;
    }
}

/*Insere um evento missao na lef.*/
void insere_missao(int id_missao, mundo_t *mundo, lef_t *l){
    evento_t eve_missao;
    eve_missao.tipo = TIPO_MISSAO;
    eve_missao.dado1 = id_missao;
    eve_missao.dado2 = 0;
    eve_missao.tempo = aleat(mundo->tempo_atual, FIM_DO_MUNDO);
    adiciona_ordem_lef(l, &eve_missao);
}

/*Trata o evento chegada e retorna uma struct mensagem com 
 * informações do que aconteceu:
 * -1 = DESISTE
 * 0 = ENTRA
 * [1, 2, ...] = Posicao na fila */
void chegada(evento_t *eve, mundo_t *mundo, lef_t *l, mensagem_t *msg){
    heroi_t *heroi = mundo->herois[eve->dado1];
    local_t *local = mundo->locais[eve->dado2];
    int tpl;

    msg->tipo = TIPO_CHEGADA;

    if (local->lot_max == cardinalidade_cjt(local->equipe)){
        if (heroi->paciencia/4 - tamanho_fila(local->fila) > 0){
            insere_fila(local->fila, heroi->id);
            msg->dado = tamanho_fila(local->fila);
        }
        else{
            msg->dado = -1;
            insere_evento_saida(heroi->id, local->id, mundo->tempo_atual, l);
        }
    }
    else{
        msg->dado = 0;
        tpl = gera_tempo_permanencia(heroi->paciencia);
        insere_cjt(local->equipe, heroi->id);
        insere_evento_saida(heroi->id, local->id, mundo->tempo_atual+tpl, l);
    }
}

/*Trata o evento de saida e retorna struct de mensagem com informacoes
 * da saida:
 * -1 = somente saida
 *  [0, 1, ...] = ID do heroi da fila que vai entrar.*/
void saida(evento_t *eve, mundo_t *mundo, lef_t *l, mensagem_t *msg){
    heroi_t *heroi = mundo->herois[eve->dado1];
    local_t *local = mundo->locais[eve->dado2];
    int id_heroi_fila, novo_local, tempo, desistiu;

    msg->tipo = TIPO_SAIDA;
    msg->dado = -1;
    desistiu = retira_cjt(local->equipe, heroi->id);
    novo_local = aleat(0, mundo->num_locais-1);
    tempo = gera_tempo_deslocamento(heroi, local, mundo->locais[novo_local])/15;

    if (!vazia_fila(local->fila) && desistiu){
        retira_fila(local->fila, &id_heroi_fila);
        insere_evento_chegada(id_heroi_fila, local->id, mundo->tempo_atual, l, INICIO);
        msg->dado = id_heroi_fila;
    }
    insere_evento_chegada(heroi->id, novo_local, mundo->tempo_atual+tempo, l, ORDEM);
}

/*Trata o evento missao e retorna struct mensagem com informacoes da missao
 * sendo nos indices do vetor:
 * [0] = conjunto da missao
 * [1, N_LOCAIS] = Equipes 
 * [dado] = Equipe escolhida (Se for NULL entao nenhuma equipe 
 * e valida.)*/
void missao(evento_t *eve, mundo_t *mundo, lef_t *l, mensagem_t *msg_missao){
    msg_missao->tipo = TIPO_MISSAO;
    msg_missao->missao[0] = cria_subcjt_cjt(mundo->todas_habs, aleat(3, 6));
    equipe_escolhida(mundo, msg_missao);

    if (msg_missao->dado != -1){
        aumenta_xp_equipe(mundo, msg_missao->dado-1);
        return;
    }
    insere_missao(eve->dado1, mundo, l);
}

/*Gera os herois e os retorna em um vetor.*/
heroi_t **cria_herois(mundo_t *mundo){
    int i;
    heroi_t **vet_herois = malloc(sizeof(heroi_t*)*mundo->num_herois);

    for (i = 0; i < mundo->num_herois; i++){
        vet_herois[i] = malloc(sizeof(heroi_t));
        vet_herois[i]->id = i;
        vet_herois[i]->xp = 0;
        vet_herois[i]->paciencia = aleat(0, 100);
        vet_herois[i]->idade = aleat(18, 100);
        vet_herois[i]->habs = cria_subcjt_cjt(mundo->todas_habs, aleat(2, MAX_HABS));
    }

    return vet_herois;
}

/*Gera os locais e os retorna em um vetor.*/
local_t **cria_locais(mundo_t *mundo){
    int i;
    local_t **vet_locais = malloc(sizeof(local_t*)*mundo->num_locais);

    for (i = 0; i < mundo->num_locais; i++){
        vet_locais[i] = malloc(sizeof(local_t));
        vet_locais[i]->id = i;
        vet_locais[i]->lot_max = aleat(5, MAX_LOT);
        vet_locais[i]->coord.x = aleat(0, mundo->tam_mundo-1);
        vet_locais[i]->coord.y = aleat(0, mundo->tam_mundo-1);
        vet_locais[i]->equipe = cria_cjt(vet_locais[i]->lot_max);
        vet_locais[i]->fila = cria_fila();
    }

    return vet_locais;
}

/*Retorna um mundo com especificacoes predefinidas.*/
mundo_t *cria_mundo(){
    mundo_t *mundo = malloc(sizeof(mundo_t));
    int i;

    mundo->todas_habs = cria_cjt(N_HABS);
    for (i = 0; i < N_HABS; i++){
        insere_cjt(mundo->todas_habs, i);
    }
    mundo->tempo_atual = 0;
    mundo->tam_mundo = 20000;
    mundo->num_herois = N_HEROIS;
    mundo->herois = cria_herois(mundo);
    mundo->num_locais = N_LOCAIS;
    mundo->locais = cria_locais(mundo);

    return mundo; 
}

/*Desaloca toda a memoria de uma struct mundo_t.*/
mundo_t *ragnarok(mundo_t *mundo){
    int i;

    for (i = 0; i < mundo->num_herois; i++){
        destroi_cjt(mundo->herois[i]->habs);
        free(mundo->herois[i]);
    }
    free(mundo->herois);
    for (i = 0; i < mundo->num_locais; i++){
        destroi_cjt(mundo->locais[i]->equipe);
        destroi_fila(mundo->locais[i]->fila);
        free(mundo->locais[i]);
    }   
    free(mundo->locais);
    destroi_cjt(mundo->todas_habs);

    free(mundo);
    return NULL;
}

/*Cria os primeiros eventos com dados predefinidos.*/
void primeiros_eventos(mundo_t *mundo, lef_t *l){
    int i, heroi, local;
    evento_t eve_fim;

    /*Gera eventos de chegada para todos os herois.*/
    for (i = 0; i < mundo->num_herois;  i++){
        heroi = mundo->herois[i]->id;
        local = aleat(0, mundo->num_locais-1);
        insere_evento_chegada(heroi, local, aleat(0, 96*7), l, ORDEM);
    }
    for (i = 0; i < FIM_DO_MUNDO/100; i++){
        insere_missao(i, mundo, l);
    }

    eve_fim.tempo = FIM_DO_MUNDO;
    eve_fim.tipo = TIPO_FIM;
    eve_fim.dado1 = 0;
    eve_fim.dado2 = 0;

    adiciona_ordem_lef(l, &eve_fim);   
}

/*Imprime o resultado da missao.*/
void aux_imprime_missao(mundo_t *mundo, mensagem_t *msg, int tempo, int id){
    int i;

    printf ("%6d:MISSAO %3d HAB_REQ ", tempo, id); 
    imprime_cjt(msg->missao[0]);
    for (i = 0; i < N_LOCAIS; i++) {
        printf ("%6d:MISSAO %3d HAB_EQL %d:", tempo, id, i); 
        imprime_cjt(msg->missao[i+1]);
    }
    if (msg->dado == -1){
        printf ("%6d:MISSAO %3d IMPOSSIVEL\n", tempo, id); 
        return;
    }
    printf ("%6d:MISSAO %3d HER_EQS %d:", tempo, id, msg->dado-1); 
    imprime_cjt(mundo->locais[msg->dado-1]->equipe);
}

/*Formata a saida de acordo com os dados da struct msg.*/
void imprime_evento(mundo_t *mundo, evento_t *evento, mensagem_t *msg){
    int i;
    int tempo = evento->tempo;
    int id = evento->dado1;
    int local = evento->dado2;
    int tam_equipe = cardinalidade_cjt(mundo->locais[local]->equipe);
    int lot_max = mundo->locais[local]->lot_max;

    switch (msg->tipo){
        case TIPO_CHEGADA:
            printf ("%6d:CHEGA HEROI %2d Local %d (%2d/%2d),", tempo, id, local, tam_equipe, lot_max);
            if (msg->dado == 0)
                printf (" ENTRA\n");
            else if (msg->dado == -1)
                printf (" DESISTE\n");
            else
                printf (" FILA %d\n", msg->dado);
            break;
        case TIPO_SAIDA:
            printf ("%6d:SAIDA HEROI %2d Local %d (%2d/%2d),", tempo, id, local, tam_equipe, lot_max); 
            if (msg->dado != -1)
                printf (" REMOVE FILA HEROI %2d\n", msg->dado);
            else 
                printf ("\n");
            break;
        case TIPO_MISSAO:
            aux_imprime_missao(mundo, msg, tempo, id);
            break;
        case TIPO_FIM:
            printf ("%6d:FIM\n", tempo);
            for (i = 0; i < mundo->num_herois; i++){
                id = mundo->herois[i]->id;
                printf ("HEROI %2d EXPERIENCIA %2d\n", id, mundo->herois[i]->xp);
            }
            break;
        default:
            break;
    }
}

/*Desaloca a struct mensagem_t.*/
void desaloca_missao(mensagem_t *msg){
    int i;

    for (i = 0; i <= N_LOCAIS; i++) {
        msg->missao[i] = destroi_cjt(msg->missao[i]);
    }
}

int main(){
    mundo_t *mundo = cria_mundo();
    lef_t *l = cria_lef();
    evento_t *evento;
    mensagem_t *mensagem = malloc(sizeof(mensagem_t));
    mensagem->missao = malloc(sizeof(conjunto_t*)*N_LOCAIS+1);
    int fim_nao_chegou = 1;

    srand(0);
    primeiros_eventos(mundo, l);

    while (fim_nao_chegou){
        evento = obtem_primeiro_lef(l);
        mundo->tempo_atual = evento->tempo;
        switch (evento->tipo){
            case TIPO_CHEGADA:
                chegada(evento, mundo, l, mensagem);
                break;
            case TIPO_SAIDA:
                saida(evento, mundo, l, mensagem);
                break;
            case TIPO_MISSAO:
                missao(evento, mundo, l, mensagem);
                break;
            case TIPO_FIM:
                mensagem->tipo = TIPO_FIM;
                fim_nao_chegou = 0;
                break;
            default:
                break;
        }
        imprime_evento(mundo, evento, mensagem);
        if (mensagem->tipo == TIPO_MISSAO)
            desaloca_missao(mensagem);
        free(evento);
    }
    free(mensagem->missao);
    free(mensagem);
    destroi_lef(l);
    ragnarok(mundo);

    return 0;
}
