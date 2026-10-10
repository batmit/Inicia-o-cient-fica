#ifndef BaB_H
#define BaB_H

typedef struct {
    double **custo;
    int n;
    int *rota;
    int *melhorRota;
    unsigned char *visitado;
    double melhorCusto;
    int erroMemoria;
} BB;


double branchAndBoundCaminhao(double **custo, int n, int *melhorRota);




#endif