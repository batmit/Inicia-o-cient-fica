#ifndef CV_1_H
#define CV_1_H

#define CIDADES 8

typedef struct cidade Cidade;
void cacheiroViajante(Cidade *vetorCidades, int *caminho, double *n, int pos, int *resultadoFinal);
double distanciaEuclidiana(Cidade *vetorCidades, int *caminho);

#endif