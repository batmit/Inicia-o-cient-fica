#ifndef CV_2_H
#define CV_2_H
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include<string.h>
#define CIDADES 27// altere conforme a instância



typedef struct cidade {
    double latitude, longitude;
    int visitado;
}Cidade;

typedef struct {

    Cidade *cidades;

    int *podeDrone;

    int n;

    double fatorCaminhao;
    double fatorDrone;

    double maxFly;

} InstanciaTSPD;

//A operção vai substituir o antigo vetor de inteiros de caminho 

typedef struct {
    int origem;
    int destino;
    int drone; // -1 se não houver drone
} Operacao;

typedef struct {

    Operacao *operacoes;

    int quantidadeOperacoes;

    double custoTotal;

} SolucaoTSPD;

double **criarMatrizDistancia(Cidade *cidades, int n, double fator);
double custoOperacaoTSPD(int i, int j, int k, double **matrizCaminhao, double **matrizDrone);


double distanciaEuclidiana(double x1, double y1, double x2, double y2);



double vizinhoMaisProximo(double **matrizDistancia, int *resultadoFinal, int posInicial, InstanciaTSPD instancia, int *visitado);

void heuristicaInsercaoBarata(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal, int nivel);

void twoopt(Cidade *vetorCidades, double **matrizDistancia, int *rota);

void inverterTrecho(int *rota, int inicio, int fim);

double calcularCusto(double **matrizDistancia, int *resultado);

SolucaoTSPD multiStart(InstanciaTSPD instancia, double **matrizCaminhao, double **matrizDrone);

SolucaoTSPD construirSolucaoTSPD( int *rota, int n, double **matrizCaminhao, double **matrizDrone, int *podeDrone);

int lerInstanciaTSPD(const char *nomeArquivo, InstanciaTSPD *inst);



void branchAndBound(
    int nivel,
    int atual,
    double custoAtual,
    double *melhorCusto,
    Operacao caminho[],
    Operacao melhorCaminho[],
    int qtdOperacoes,
    int *melhorQtdOperacoes,
    InstanciaTSPD instancia,
    double **matrizCaminhao,
    double **matrizDrone,
    int *visitado
);

#endif