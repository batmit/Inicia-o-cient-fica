#include <stdio.h>
#include<math.h>
#include "tspd.h"
double distanciaEuclidiana(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - y1) * (x1 - y1) + (x2 - y2) * (x2 - y2));
}


void branchAndBound( int nivel, int atual, double custoAtual,double *melhorCusto, Operacao caminho[], Operacao melhorCaminho[], int qtdOperacoes, int *melhorQtdOperacoes, InstanciaTSPD instancia,
    double **matrizCaminhao,
    double **matrizDrone,
    int *visitado
) {

    if (nivel == instancia.n) {

        double custoFinal = custoAtual + matrizCaminhao[atual][0];

        if (custoFinal < *melhorCusto) {

            *melhorCusto = custoFinal;

            caminho[qtdOperacoes].origem = atual;
            caminho[qtdOperacoes].destino = 0;
            caminho[qtdOperacoes].drone = -1;

            *melhorQtdOperacoes = qtdOperacoes + 1;

            memcpy(
                melhorCaminho,
                caminho,
                (*melhorQtdOperacoes) * sizeof(Operacao)
            );
        }

        return;
    }

    if (custoAtual >= *melhorCusto) {
        return;
    }

    for (int prox = 1; prox < instancia.n; prox++) {

        if (!visitado[prox]) {

            /*
                CASO 1:
                O caminhão vai diretamente de atual até prox.
            */

            visitado[prox] = 1;

            caminho[qtdOperacoes].origem = atual;
            caminho[qtdOperacoes].destino = prox;
            caminho[qtdOperacoes].drone = -1;

            branchAndBound(
                nivel + 1,
                prox,
                custoAtual + matrizCaminhao[atual][prox],
                melhorCusto,
                caminho,
                melhorCaminho,
                qtdOperacoes + 1,
                melhorQtdOperacoes,
                instancia,
                matrizCaminhao,
                matrizDrone,
                visitado);


            /*
                CASO 2:
                O caminhão vai de atual até prox,
                e o drone atende a cidade k no meio.
            */

            for (int k = 1; k < instancia.n; k++) {

                if (!visitado[k]  && k!=prox && instancia.podeDrone[k]) {

                    visitado[k] = 1;

                    double custoOp = custoOperacaoTSPD(
                        atual,
                        prox,
                        k,
                        matrizCaminhao,
                        matrizDrone
                    );

                    caminho[qtdOperacoes].origem = atual;
                    caminho[qtdOperacoes].destino = prox;
                    caminho[qtdOperacoes].drone = k;

                    branchAndBound(
                        nivel + 2,
                        prox,
                        custoAtual + custoOp,
                        melhorCusto,
                        caminho,
                        melhorCaminho,
                        qtdOperacoes + 1,
                        melhorQtdOperacoes,
                        instancia,
                        matrizCaminhao,
                        matrizDrone,
                        visitado
                    );

                    visitado[k] = 0;
                }
            }
            visitado[prox] = 0;
        }
    }
}


double vizinhoMaisProximo(double **matrizDistancia, int *resultadoFinal, int posInicial, InstanciaTSPD instancia, int *visitado) {
    double soma = 0;
    int atual   = resultadoFinal[posInicial - 1];

    for (int pos = posInicial; pos < instancia.n; pos++) {
        double distancia = DBL_MAX;
        int cidade       = -1;

        for (int j = 1; j < instancia.n; j++) {
            if (matrizDistancia[atual][j] < distancia &&
                visitado[j] == 0) {
                distancia = matrizDistancia[atual][j];
                cidade    = j;
            }
        }

        if (cidade == -1) break;

        resultadoFinal[pos]          = cidade;
        soma                        += distancia;
        visitado[cidade] = 1;
        atual                        = cidade;
    }

    soma += matrizDistancia[atual][0];
    return soma;
}

SolucaoTSPD multiStart(InstanciaTSPD instancia, double **matrizCaminhao, double **matrizDrone) {
    int *resultadoFinal = malloc(instancia.n * (sizeof(int)));
    resultadoFinal[0] = 0;
    int *visitado = calloc(instancia.n, sizeof(int));
    visitado[0] = 1;
    vizinhoMaisProximo(matrizCaminhao, resultadoFinal, 1,instancia , visitado);
    SolucaoTSPD atual = construirSolucaoTSPD( resultadoFinal, instancia.n, matrizCaminhao, matrizDrone,instancia.podeDrone);
    free(resultadoFinal);
    free(visitado);
    return atual;
}


void inverterTrecho(int *rota, int inicio, int fim) {
    while (inicio < fim) {
        int temp       = rota[inicio];
        rota[inicio++] = rota[fim];
        rota[fim--]    = temp;
    }
}

double calcularCusto(double **matrizDistancia, int *resultado) {
    double custo = 0;
    for (int i = 0; i < CIDADES - 1; i++)
        custo += matrizDistancia[resultado[i]][resultado[i + 1]];
    custo += matrizDistancia[resultado[CIDADES - 1]][resultado[0]];
    return custo;
}


double **criarMatrizDistancia(Cidade *cidades, int n, double fator) {
    double **matriz = malloc(n * sizeof(double *));

    for (int i = 0; i < n; i++) {
        matriz[i] = malloc(n * sizeof(double));

        for (int j = 0; j < n; j++) {
            matriz[i][j] = distanciaEuclidiana(cidades[i].latitude, cidades[j].latitude, cidades[i].longitude, cidades[j].longitude) * fator; //você multiplica por um fator
        }
    }

    return matriz;
}

double custoOperacaoTSPD(int i, int j, int k, double **matrizCaminhao, double **matrizDrone) {
    double custoCaminhao = matrizCaminhao[i][j];

    if (k == -1) {
        return custoCaminhao;
    }

    double custoDrone = matrizDrone[i][k] + matrizDrone[k][j];

    if (custoCaminhao > custoDrone) {
        return custoCaminhao;
    }

    return custoDrone;
}


SolucaoTSPD construirSolucaoTSPD( int *rota, int n, double **matrizCaminhao, double **matrizDrone, int *podeDrone) {

    SolucaoTSPD solucao;

    solucao.operacoes = malloc(n * sizeof(Operacao));

    solucao.quantidadeOperacoes = 0;

    solucao.custoTotal = 0.0;
    int ultimoCaminhao = 0;
    int i = 0;

    while(i < n - 1) {

        int origem = rota[i];

        int cliente = rota[i + 1];

        int destino;

        if(i + 2 < n) {
            destino = rota[i + 2];
        }
        else {
            destino = 0;
        }

        int usarDrone = 0;

        if(podeDrone[cliente]) {

            double semDrone =
                matrizCaminhao[origem][cliente]
              + matrizCaminhao[cliente][destino];

            double comDrone =
                custoOperacaoTSPD(
                    origem,
                    destino,
                    cliente,
                    matrizCaminhao,
                    matrizDrone
                );

            if(comDrone < semDrone) {
                usarDrone = 1;
            }
        }

        if(usarDrone) {

            solucao.operacoes[solucao.quantidadeOperacoes].origem = origem;

            solucao.operacoes[solucao.quantidadeOperacoes].destino = destino;

            solucao.operacoes[solucao.quantidadeOperacoes].drone = cliente;

            solucao.custoTotal += custoOperacaoTSPD( origem, destino, cliente, matrizCaminhao, matrizDrone );

            solucao.quantidadeOperacoes++;
            ultimoCaminhao = destino;
            i += 2;
        }
        else {

            solucao.operacoes[ solucao.quantidadeOperacoes ].origem = origem;

            solucao.operacoes[ solucao.quantidadeOperacoes ].destino = cliente;

            solucao.operacoes[ solucao.quantidadeOperacoes ].drone = -1;

            solucao.custoTotal += matrizCaminhao[origem][cliente];

            solucao.quantidadeOperacoes++;

            i++;
            ultimoCaminhao = cliente;

        }
    }
    if (ultimoCaminhao != 0) {
        solucao.custoTotal += matrizCaminhao[ultimoCaminhao][0];
    }

    return solucao;
}
