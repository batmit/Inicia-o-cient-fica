#include <stdio.h>
#include "tspd.h"

//Criar uma função main que excecuta as operações e recebe apenas o nome do arquivo
//Automatizar todo o processo do arquivo
//Personalizar entradas e pensar na entrada de dados

int main(){


    InstanciaTSPD instancia;
    char nomeArquivo[256];

    printf("Digite o nome do arquivo da instancia: ");
    scanf("%255s", nomeArquivo);

    if (!lerInstanciaTSPD(nomeArquivo, &instancia)) {

        printf("Falha ao carregar a instancia.\n");
        return 1;
    }


    double **matrizCaminhao = criarMatrizDistancia(instancia.cidades, instancia.n, instancia.fatorCaminhao);
    double **matrizDrone = criarMatrizDistancia(instancia.cidades, instancia.n, instancia.fatorDrone);

    SolucaoTSPD solucaoInicial = multiStart(instancia, matrizCaminhao, matrizDrone);


    double melhorCusto = solucaoInicial.custoTotal;

    Operacao *caminho = malloc(instancia.n * sizeof(Operacao));
    Operacao *melhorCaminho = malloc(instancia.n * sizeof(Operacao));   

    int melhorQtdOperacoes = solucaoInicial.quantidadeOperacoes;

    memcpy(
        melhorCaminho,
        solucaoInicial.operacoes,
        melhorQtdOperacoes * sizeof(Operacao)
    );

    int *visitado = calloc(instancia.n, sizeof(int));

    visitado[0] = 1;

    branchAndBound(
        1,
        0,
        0.0,
        &melhorCusto,
        caminho,
        melhorCaminho,
        0,
        &melhorQtdOperacoes,
        instancia,
        matrizCaminhao,
        matrizDrone,
        visitado
    );

    printf("Melhor custo TSP-D: %.6lf\n", melhorCusto);

    printf("Operacoes:\n");

    for (int i = 0; i < instancia.n - 1; i++) {

        if (melhorCaminho[i].drone == -1) {
            printf(
                "Caminhao: %d -> %d\n",
                melhorCaminho[i].origem,
                melhorCaminho[i].destino
            );
        } else {
            printf(
                "Caminhao: %d -> %d | Drone atende: %d\n",
                melhorCaminho[i].origem,
                melhorCaminho[i].destino,
                melhorCaminho[i].drone
            );
        }
    }


    for(int i = 0; i < instancia.n; i++){

        free(matrizCaminhao[i]);
        free(matrizDrone[i]);

    }

    free(matrizCaminhao);
    free(matrizDrone);
    free(caminho);
    free(melhorCaminho);
    free(visitado);


    return 0;


}