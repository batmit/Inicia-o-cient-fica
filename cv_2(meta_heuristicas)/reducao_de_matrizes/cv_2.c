#include <stdio.h>
#include <stdlib.h>
#include "cv_2.h"
#include <math.h>
#include <float.h>
#include <string.h>

struct cidade {
    double latitude, longitude;
    int visitado;
};

int main() {

    Cidade *vetorCidades = malloc(CIDADES * sizeof(Cidade));
    int *caminho         = malloc(CIDADES * sizeof(int));
    int *resultadoFinal  = malloc(CIDADES * sizeof(int));

    // vizinhosOrdenados[i][j] = j-ésimo vizinho mais próximo da cidade i
    int **vizinhosOrdenados = malloc(CIDADES * sizeof(int *));
    for (int i = 0; i < CIDADES; i++) {
        vizinhosOrdenados[i] = malloc((CIDADES - 1) * sizeof(int));
    }

    for (int i = 0; i < CIDADES; i++) {
        resultadoFinal[i]      = 0;
        caminho[i]             = -1;
        vetorCidades[i].visitado = 0;
        scanf("%lf", &vetorCidades[i].latitude);
        scanf("%lf", &vetorCidades[i].longitude);
    }

    // Matriz de distâncias
    double **matrizDistancia = malloc(CIDADES * sizeof(double *));
    for (int i = 0; i < CIDADES; i++) {
        matrizDistancia[i] = malloc(CIDADES * sizeof(double));
        for (int j = 0; j < CIDADES; j++) {
            matrizDistancia[i][j] = distanciaEuclidiana(
                vetorCidades[i].latitude,  vetorCidades[j].latitude,
                vetorCidades[i].longitude, vetorCidades[j].longitude
            );
        }
        matrizDistancia[i][i] = DBL_MAX; // diagonal infinita
    }

    ordenarVizinhos(vizinhosOrdenados, vetorCidades, matrizDistancia);

    caminho[0] = 0;
    vetorCidades[0].visitado = 1;

    int *resultadoParcial = malloc(sizeof(int) * CIDADES);
    double melhorCusto = multiStart(vetorCidades, matrizDistancia, resultadoParcial);

    // Inicializa resultadoFinal com a melhor rota da heurística
    for (int i = 0; i < CIDADES; i++) {
        resultadoFinal[i]        = resultadoParcial[i];
        vetorCidades[i].visitado = 0;
    }
    vetorCidades[0].visitado = 1;

    // -----------------------------------------------------------------
    // Pool de matrizes: uma por nível da árvore — zero malloc no B&B
    // A profundidade máxima da recursão é CIDADES, então CIDADES
    // matrizes são suficientes para cobrir todos os níveis.
    // -----------------------------------------------------------------
    double ***pool = malloc(CIDADES * sizeof(double **));
    for (int n = 0; n < CIDADES; n++) {
        pool[n] = malloc(CIDADES * sizeof(double *));
        for (int i = 0; i < CIDADES; i++) {
            pool[n][i] = malloc(CIDADES * sizeof(double));
        }
    }

    // Preenche o nível 0 do pool com a matriz reduzida (raiz da árvore)
    for (int i = 0; i < CIDADES; i++)
        for (int j = 0; j < CIDADES; j++)
            pool[0][i][j] = matrizDistancia[i][j];

    double limiteRaiz = reduzirMatriz(pool[0]);

    branchAndBound( vetorCidades, 1, caminho, 0, matrizDistancia, &melhorCusto, resultadoFinal, pool, limiteRaiz, vizinhosOrdenados);

    printf("\nMenor distancia encontrada: %.6lf\n", melhorCusto);
    printf("\nCaminho: ");
    for (int i = 0; i < CIDADES; i++)
        printf("%d ", resultadoFinal[i]);
    printf("\n");

    // Liberação
    free(resultadoParcial);
    free(resultadoFinal);
    free(caminho);
    free(vetorCidades);

    for (int i = 0; i < CIDADES; i++)
        free(matrizDistancia[i]);
    free(matrizDistancia);

    for (int n = 0; n < CIDADES; n++) {
        for (int i = 0; i < CIDADES; i++)
            free(pool[n][i]);
        free(pool[n]);
    }
    free(pool);

    for (int i = 0; i < CIDADES; i++)
        free(vizinhosOrdenados[i]);
    free(vizinhosOrdenados);

    return 0;
}

// ---------------------------------------------------------------------

double distanciaEuclidiana(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - y1) * (x1 - y1) + (x2 - y2) * (x2 - y2));
}

// ---------------------------------------------------------------------
// Branch and Bound com pool de matrizes e vizinhos ordenados
//
// pool[nivel] é a matriz disponível para este nível da árvore.
// Em vez de malloc/free a cada nó, copiamos pool[nivel-1] → pool[nivel],
// modificamos pool[nivel] e passamos para o filho. Ao retornar,
// pool[nivel] será sobrescrito pela próxima iteração — sem custo extra.
// ---------------------------------------------------------------------
void branchAndBound(
    Cidade *vetorCidades, int nivel, int caminho[],
    double custoAtual, double **matrizDistancia,
    double *melhorCusto, int resultadoFinal[],
    double ***pool, double limiteAtual,
    int **vizinhosOrdenados) {

    if (nivel == CIDADES) {
        double custoTotal = custoAtual + matrizDistancia[caminho[nivel - 1]][caminho[0]];
        if (custoTotal < *melhorCusto) {
            *melhorCusto = custoTotal;
            memcpy(resultadoFinal, caminho, CIDADES * sizeof(int));
        }
        return;
    }

    //Identifica qual a ultima cidade utilizada
    int ultimaCidade = caminho[nivel - 1];

    // Itera candidatos na ordem dos vizinhos mais próximos de ultimaCidade.
    // Isso garante que ramos promissores são explorados primeiro,
    // produzindo boas soluções cedo e maximizando a poda.
    for (int i = 0; i < CIDADES - 1; i++) {
        int candidato = vizinhosOrdenados[ultimaCidade][i];

        if (vetorCidades[candidato].visitado == 0) {
            
            //Implementação a ser analisada---------
            double novoCusto = custoAtual + matrizDistancia[ultimaCidade][candidato];

            if (novoCusto >= *melhorCusto) {
                continue;
            }
            //---------------------------------------

            // Copia pool[nivel-1] → pool[nivel] sem malloc
            for (int r = 0; r < CIDADES; r++)
                memcpy(pool[nivel][r], pool[nivel - 1][r], CIDADES * sizeof(double));

            // Elimina linha (ultimaCidade já saiu) e coluna (candidato já chegou)
            for (int k = 0; k < CIDADES; k++) {
                pool[nivel][ultimaCidade][k] = DBL_MAX;
                pool[nivel][k][candidato]    = DBL_MAX;
            }

            // Bloqueia retorno prematuro à origem, 
            //impedindo que retorne a cidade 0 sem passar primeiro por todas as outras
            if(nivel < CIDADES - 1){
                pool[nivel][candidato][caminho[0]] = DBL_MAX;


            }

            // Custo da aresta lido ANTES de reduzir (valor residual correto)
            //o valor que foi reduzido de toda a linha

            double custaAresta = pool[nivel - 1][ultimaCidade][candidato];
            double reducao     = reduzirMatriz(pool[nivel]);
            //o limite atual(tudo que já foi calculado dos valores anteriores)+
            //o custo da Aresta que é o valor que foi subtraído em toda a linha +
            //custo mínimo, da nova matriz já reduzida

            double novoLimite  = limiteAtual + custaAresta + reducao;

            // Custo real acumulado (usando distâncias originais)
            double novoCusto = custoAtual + matrizDistancia[ultimaCidade][candidato];

            if (novoLimite < *melhorCusto) {
                caminho[nivel] = candidato;
                vetorCidades[candidato].visitado = 1;

                branchAndBound( vetorCidades, nivel + 1, caminho, novoCusto, matrizDistancia, melhorCusto, resultadoFinal, pool, novoLimite, vizinhosOrdenados);

                vetorCidades[candidato].visitado = 0;
                caminho[nivel] = -1;
            }
            // Sem liberarMatriz — pool[nivel] será reutilizado na próxima iteração
        }
    }
}

// ---------------------------------------------------------------------

double reduzirMatriz(double **m) {
    double total = 0;

    for (int i = 0; i < CIDADES; i++) {
        double minVal = DBL_MAX;
        for (int j = 0; j < CIDADES; j++)
            if (m[i][j] < minVal) minVal = m[i][j];
        if (minVal == DBL_MAX || minVal == 0) continue;
        for (int j = 0; j < CIDADES; j++)
            if (m[i][j] != DBL_MAX) m[i][j] -= minVal;
        total += minVal;
    }

    for (int j = 0; j < CIDADES; j++) {
        double minVal = DBL_MAX;
        for (int i = 0; i < CIDADES; i++)
            if (m[i][j] < minVal) minVal = m[i][j];
        if (minVal == DBL_MAX || minVal == 0) continue;
        for (int i = 0; i < CIDADES; i++)
            if (m[i][j] != DBL_MAX) m[i][j] -= minVal;
        total += minVal;
    }

    return total;
}

// ---------------------------------------------------------------------

void ordenarVizinhos(int **vizinhosOrdenados, Cidade *vetorCidades, double **matrizDistancia) {
    for (int i = 0; i < CIDADES; i++) {

        // Marca cidade i como visitada para não escolhê-la como própria vizinha
        vetorCidades[i].visitado = 1;

        for (int j = 0; j < CIDADES - 1; j++) {

            // Acha o primeiro não visitado como referência inicial
            int nearer = -1;
            for (int w = 0; w < CIDADES; w++) {
                if (vetorCidades[w].visitado == 0) {
                    nearer = w;
                    break;
                }
            }

            // Busca o mais próximo de i entre os não visitados
            for (int z = 0; z < CIDADES; z++) {
                if (vetorCidades[z].visitado == 0 &&
                    matrizDistancia[i][z] < matrizDistancia[i][nearer]) {
                    nearer = z;
                }
            }

            vizinhosOrdenados[i][j] = nearer;
            vetorCidades[nearer].visitado = 1;
        }

        // Reseta visitados para a próxima cidade i
        for (int j = 0; j < CIDADES; j++)
            vetorCidades[j].visitado = 0;
    }
}

// ---------------------------------------------------------------------

double vizinhoMaisProximo(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal, int posInicial) {
    double soma = 0;
    int atual   = resultadoFinal[posInicial - 1];

    for (int pos = posInicial; pos < CIDADES; pos++) {
        double distancia = DBL_MAX;
        int cidade       = -1;

        for (int j = 1; j < CIDADES; j++) {
            if (matrizDistancia[atual][j] < distancia &&
                vetorCidades[j].visitado == 0) {
                distancia = matrizDistancia[atual][j];
                cidade    = j;
            }
        }

        if (cidade == -1) break;

        resultadoFinal[pos]          = cidade;
        soma                        += distancia;
        vetorCidades[cidade].visitado = 1;
        atual                        = cidade;
    }

    soma += matrizDistancia[atual][0];
    return soma;
}

// ---------------------------------------------------------------------

double multiStart(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal) {
    int *rotaAtual = malloc(sizeof(int) * CIDADES);
    int *vetAux    = malloc(sizeof(int) * CIDADES);
    vetAux[0]    = 0;
    rotaAtual[0] = 0;
    vetorCidades[0].visitado = 1;
    double melhorCusto = DBL_MAX;

    for (int i = 1; i < CIDADES; i++) {

        for (int j = 1; j < CIDADES; j++) {
            vetAux[j]    = -1;
            rotaAtual[j] = -1;
            vetorCidades[j].visitado = 0;
        }
        vetAux[1]    = i;
        rotaAtual[1] = i;
        vetorCidades[i].visitado = 1;

        vizinhoMaisProximo(vetorCidades, matrizDistancia, rotaAtual, 2);
        twoopt(vetorCidades, matrizDistancia, rotaAtual);
        double vmP = calcularCusto(matrizDistancia, rotaAtual);

        for (int j = 1; j < CIDADES; j++)
            if (j != i) 
            vetorCidades[j].visitado = 0;

        heuristicaInsercaoBarata(vetorCidades, matrizDistancia, vetAux, 2);
        twoopt(vetorCidades, matrizDistancia, vetAux);
        double insercao = calcularCusto(matrizDistancia, vetAux);

        double custo;
        int *melhorRota;
        if (vmP < insercao) { 
            custo = vmP;     
            melhorRota = rotaAtual; 
        }
        else{ 
            custo = insercao; 
            melhorRota = vetAux;   
        }

        if (custo < melhorCusto) {
            melhorCusto = custo;
            memcpy(resultadoFinal, melhorRota, CIDADES * sizeof(int));
        }
    }

    free(rotaAtual);
    free(vetAux);
    return melhorCusto;
}

// ---------------------------------------------------------------------

void twoopt(Cidade *vetorCidades, double **matrizDistancia, int *rota) {
    int melhorou = 1;
    while (melhorou) {
        melhorou = 0;
        for (int i = 1; i < CIDADES - 1; i++) {
            for (int j = i + 1; j < CIDADES; j++) {
                int a = rota[i - 1], b = rota[i];
                int c = rota[j], d = rota[(j + 1) % CIDADES];

                if (matrizDistancia[a][c] + matrizDistancia[b][d] < matrizDistancia[a][b] + matrizDistancia[c][d]) {
                    inverterTrecho(rota, i, j);
                    melhorou = 1;
                }
            }
        }
    }
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

// ---------------------------------------------------------------------

void heuristicaInsercaoBarata(Cidade *vetorCidades, double **matrizDistancia,
                               int *resultadoFinal, int nivel) {
    int tamCiclo = nivel;

    while (tamCiclo < CIDADES) {
        int melhorCidade   = -1;
        int melhorPosicao  = -1;
        double melhorDelta = DBL_MAX;

        for (int k = 0; k < CIDADES; k++) {
            if (vetorCidades[k].visitado == 0) {
                for (int pos = 0; pos < tamCiclo; pos++) {
                    int ci = resultadoFinal[pos];
                    int cj = (pos == tamCiclo - 1) ? resultadoFinal[0] : resultadoFinal[pos + 1];

                    double delta = matrizDistancia[ci][k] + matrizDistancia[k][cj] - matrizDistancia[ci][cj];

                    if (delta < melhorDelta) {
                        melhorDelta   = delta;
                        melhorCidade  = k;
                        melhorPosicao = pos;
                    }
                }
            }
        }

        for (int m = tamCiclo; m > melhorPosicao + 1; m--)
            resultadoFinal[m] = resultadoFinal[m - 1];

        resultadoFinal[melhorPosicao + 1]      = melhorCidade;
        vetorCidades[melhorCidade].visitado    = 1;
        tamCiclo++;
    }
}