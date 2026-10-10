#ifndef MAIN_H
#define MAIN_H
#include<stdbool.h>
typedef struct Cliente {
    int id;

    double x;
    double y;

    double peso;

    bool elegivelDrone;

    // quantidade mínima de drones necessária
    // para transportar a encomenda
    int minDrones;
}Cliente;

typedef struct Instancia
{
    int n;
    int numDrones;

    double truckSpeed;

    double batteryWeight;
    double batteryCapacity;
    double maxDischargeCurrent;
    double voltage;

    Cliente *clientes;

    double **tempoCaminhao;




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
}MissaoDrone;

typedef struct Solucao {
    RotaCaminhao caminhao;

    int numMissoes;
    MissaoDrone *missoesDrone;

    double makespan;
}Solucao;



#endif