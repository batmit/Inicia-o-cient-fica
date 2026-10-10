#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

typedef struct Cliente {
    int id;

    double x;
    double y;
    double peso;

    /* Sera calculado posteriormente. */
    bool elegivelDrone;

    /* Quantidade minima de drones necessaria para atender o cliente.
       Tambem sera calculada posteriormente. */
    int minDrones;
} Cliente;

typedef struct Instancia {
    /* Numero total de nos da instancia, incluindo o deposito (no 0). */
    int n;
    int numDrones;

    double truckSpeed;

    double batteryWeight;
    double batteryCapacity;
    double maxDischargeCurrent;
    double voltage;

    Cliente *clientes;

    /* Dados derivados: serao calculados em uma etapa posterior. */
    double **distCaminhao;
    double **tempoDrone;
} Instancia;

typedef struct RotaCaminhao {
    int *rota;
    int tamanho;
    double tempo;
} RotaCaminhao;

typedef struct MissaoDrone {
    int cliente;
    int qtdDrones;
    double tempoInicio;
    double tempoFim;
} MissaoDrone;

typedef struct Solucao {
    RotaCaminhao caminhao;

    int numMissoes;
    MissaoDrone *missoesDrone;

    double makespan;
} Solucao;

#endif
