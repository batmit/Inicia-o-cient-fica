#ifndef PDSTSPC_CV_H
#define PDSTSPC_CV_H
#include <stddef.h>

typedef struct {
    size_t id;
    double x_km, y_km, peso_kg;
    size_t minimo_drones, maximo_drones; /* 0: nenhum grupo viavel */
} Cliente;

typedef struct {
    double preparacao_min;
    size_t max_combinacao;
    double max_peso_gerador_kg; /* metadado, nao capacidade individual */
    double estrutura_kg;
    size_t numero_helices;
    double raio_helice_m, peukert;
    double altura_m, subida_ms, descida_ms, densidade_ar;
} ParametrosEnergia;

typedef struct {
    double tempo_min; /* missao completa, ceil; INFINITY se inviavel */
    double velocidade_ida_ms, velocidade_volta_ms;
    double consumo_ah; /* por bateria */
} OpcaoDrone;

typedef struct {
    size_t numero_drones, numero_nos, max_grupo;
    double velocidade_caminhao_kmh;
    double bateria_kg, capacidade_ah, corrente_max_a, tensao_v;
    ParametrosEnergia energia;
    Cliente *nos; /* indice/ID 0: deposito */
    double *distancias_caminhao_km; /* [i * numero_nos + j], Manhattan */
    double *tempos_caminhao_min;    /* [i * numero_nos + j] */
    double *distancias_drone_km;    /* [i], euclidiana deposito -> i */
    OpcaoDrone *opcoes_drone;      /* [i * (max_grupo + 1) + k], k >= 1 */
} InstanciaPDSTSPC;

typedef struct {
    size_t cliente, quantidade_drones;
    size_t *drones; /* IDs de 0 a numero_drones - 1 */
    double inicio_min, fim_min;
} MissaoDrone;

typedef struct {
    size_t *rota; /* inclui deposito no inicio e no fim */
    size_t tamanho_rota, numero_missoes;
    MissaoDrone *missoes;
    double tempo_caminhao_min, makespan_min;
} Solucao;

/* Libere antes de reutilizar estruturas que ja possuem memoria. */
void inicializar_instancia(InstanciaPDSTSPC *instancia);
void liberar_instancia(InstanciaPDSTSPC *instancia);
/* params NULL: procura other-params.txt na pasta do arquivo e em ate 2 pais. */
int ler_instancia_pdstspc(const char *arquivo, const char *params,
                         InstanciaPDSTSPC *instancia, char *erro, size_t tamanho_erro);
int ler_instancia(const char *arquivo, InstanciaPDSTSPC *instancia,
                  char *erro, size_t tamanho_erro);
int preprocessar_instancia(InstanciaPDSTSPC *instancia, char *erro, size_t tamanho_erro);
double calcular_Tij(double distancia_km, double velocidade_kmh);
double tempo_caminhao_min(const InstanciaPDSTSPC *instancia, size_t i, size_t j);
const OpcaoDrone *opcao_drone(const InstanciaPDSTSPC *instancia, size_t i, size_t k);
void imprimir_instancia(const InstanciaPDSTSPC *instancia);
void inicializar_solucao(Solucao *solucao);
void liberar_solucao(Solucao *solucao);
int heuristica(const InstanciaPDSTSPC *instancia, Solucao *solucao,
               char *erro, size_t tamanho_erro);
int validar_solucao(const InstanciaPDSTSPC *instancia, const Solucao *solucao,
                    char *erro, size_t tamanho_erro);
void imprimir_solucao(const Solucao *solucao);
#endif
