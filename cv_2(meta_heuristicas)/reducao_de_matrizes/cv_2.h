#ifndef CV_2_H
#define CV_2_H

#include <float.h>


#define CIDADES 27// altere conforme a instância



typedef struct cidade Cidade;


double distanciaEuclidiana(double x1, double y1, double x2, double y2);


double reduzirMatriz(double **m);


double vizinhoMaisProximo(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal, int posInicial);

void heuristicaInsercaoBarata(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal, int nivel);

void twoopt(Cidade *vetorCidades, double **matrizDistancia, int *rota);

void inverterTrecho(int *rota, int inicio, int fim);

double calcularCusto(double **matrizDistancia, int *resultado);

double multiStart(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal);


void ordenarVizinhos(int **vizinhosOrdenados, Cidade *vetorCidades, double **matrizDistancia);

void branchAndBound(Cidade *vetorCidades, int nivel, int caminho[],
                    double custoAtual, double **matrizDistancia,
                    double *melhorCusto, int resultadoFinal[],
                    double ***pool, double limiteAtual,
                    int **vizinhosOrdenados);

#endif