#include <stdio.h>
#include<stdlib.h>
#include "BaB.h"
#include <math.h>
#include<string.h>
#define M(i, j) matriz[(i) * n + (j)]


static double custoRota(const BB *b, const int *rota)
{
    double total = 0;
    for (int i = 0; i < b->n - 1; i++)
        total += b->custo[rota[i]][rota[i + 1]];
    return total + b->custo[rota[b->n - 1]][0];
}

// Solução viável inicial: vizinho mais próximo + melhoria 2-opt.
static void limiteSuperiorInicial(BB *b)
{
    const int n = b->n;
    unsigned char *v = calloc((size_t)n, sizeof(*v));
    if (!v) return;

    int atual = 0;
    b->melhorRota[0] = 0;
    v[0] = 1;

    for (int nivel = 1; nivel < n; nivel++) {
        int proximo = -1;
        double menor = INFINITY;
        for (int j = 1; j < n; j++) {
            if (!v[j] && b->custo[atual][j] < menor) {
                menor = b->custo[atual][j];
                proximo = j;
            }
        }
        if (proximo < 0) { free(v); return; }
        b->melhorRota[nivel] = proximo;
        v[proximo] = 1;
        atual = proximo;
    }
    b->melhorRota[n] = 0;
    b->melhorCusto = custoRota(b, b->melhorRota);
    free(v);

    // 2-opt simples: inverter trechos e reavaliar o custo do ciclo.
    // Válido também para matrizes assimétricas (custo reavaliado por inteiro).
    int melhorou = 1;
    while (melhorou) {
        melhorou = 0;
        for (int i = 1; i < n - 1 && !melhorou; i++) {
            for (int j = i + 1; j < n; j++) {
                for (int l = i, r = j; l < r; l++, r--) {
                    int tmp = b->melhorRota[l];
                    b->melhorRota[l] = b->melhorRota[r];
                    b->melhorRota[r] = tmp;
                }
                double novo = custoRota(b, b->melhorRota);
                if (novo < b->melhorCusto) {
                    b->melhorCusto = novo;
                    melhorou = 1;
                    break;
                }
                // Desfaz inversão se não houver melhora.
                for (int l = i, r = j; l < r; l++, r--) {
                    int tmp = b->melhorRota[l];
                    b->melhorRota[l] = b->melhorRota[r];
                    b->melhorRota[r] = tmp;
                }
            }
        }
    }
}


// Redução de linhas e colunas: retorna a parcela acrescentada ao limite.
static double reduzirMatriz(double *matriz, int n)
{
    double reducao = 0;
    for (int i = 0; i < n; i++) {
        double minimo = INFINITY;
        for (int j = 0; j < n; j++)
            if (M(i, j) < minimo) minimo = M(i, j);
        if (minimo != INFINITY && minimo > 0) {
            for (int j = 0; j < n; j++)
                if (M(i, j) != INFINITY) M(i, j) -= minimo;
            reducao += minimo;
        }
    }
    for (int j = 0; j < n; j++) {
        double minimo = INFINITY;
        for (int i = 0; i < n; i++)
            if (M(i, j) < minimo) minimo = M(i, j);
        if (minimo != INFINITY && minimo > 0) {
            for (int i = 0; i < n; i++)
                if (M(i, j) != INFINITY) M(i, j) -= minimo;
            reducao += minimo;
        }
    }
    return reducao;
}


static void explorar(BB *b, const double *reduzida,
                    double limite, double custoAtual,
                    int ultimo, int nivel)
{
    int n = b->n;
    for (int j = 1; j < n; j++) {
        if (b->visitado[j]) continue;

        // Arco proibido pelo estado da matriz.
        double custoReduzido = reduzida[ultimo * n + j];
        if (custoReduzido == INFINITY) continue;

        double novoCusto = custoAtual + b->custo[ultimo][j];
        if (novoCusto >= b->melhorCusto) continue;

        b->rota[nivel] = j;
        if (nivel + 1 == n) {
            // Último cliente: fechar a rota no depósito.
            double total = novoCusto + b->custo[j][0];
            if (total < b->melhorCusto) {
                b->melhorCusto = total;
                memcpy(b->melhorRota, b->rota,
                       (size_t)n * sizeof(*b->rota));
                b->melhorRota[n] = 0;
            }
            continue;
        }

        double *filha = malloc((size_t)n * n * sizeof(*filha));
        if (filha == NULL) { b->erroMemoria = 1; return; }
        memcpy(filha, reduzida, (size_t)n * n * sizeof(*filha));

        // Escolhida a aresta ultimo -> j, o vértice 'ultimo' já tem
        // saída e 'j' já tem entrada. Impedir retorno prematuro j -> 0.
        for (int col = 0; col < n; col++) filha[ultimo * n + col] = INFINITY;
        for (int lin = 0; lin < n; lin++) filha[lin * n + j] = INFINITY;
        filha[j * n + 0] = INFINITY;

        // Limite inferior de Little: limite pai + custo reduzido do arco
        // escolhido + reduções adicionais da matriz-filha.
        double novoLimite = limite + custoReduzido + reduzirMatriz(filha, n);
        if (novoLimite < b->melhorCusto) {
            b->visitado[j] = 1;
            explorar(b, filha, novoLimite, novoCusto, j, nivel + 1);
            b->visitado[j] = 0;
        }
        free(filha);
        if (b->erroMemoria) return;
    }
}


// Retorna a melhor rota 0 -> ... -> 0 em melhorRota[0..n].
double branchAndBoundCaminhao(double **custo, int n, int *melhorRota)
{
    if (!custo || !melhorRota || n <= 0) return INFINITY;
    if (n == 1) {
        melhorRota[0] = melhorRota[1] = 0;
        return 0.0;
    }

    BB b = {0};
    b.custo = custo;
    b.n = n;
    b.melhorRota = melhorRota;
    b.melhorCusto = INFINITY;
    b.rota = malloc((size_t)(n + 1) * sizeof(*b.rota));
    b.visitado = calloc((size_t)n, sizeof(*b.visitado));
    double *raiz = malloc((size_t)n * n * sizeof(*raiz));
    if (!b.rota || !b.visitado || !raiz) {
        free(b.rota); free(b.visitado); free(raiz);
        return INFINITY;
    }

    for (int i = 0; i < n; i++) {
        if (!custo[i]) {
            free(b.rota); free(b.visitado); free(raiz);
            return INFINITY;
        }
        for (int j = 0; j < n; j++) {
            raiz[i * n + j] = (i == j) ? INFINITY : custo[i][j];
        }
    }

    limiteSuperiorInicial(&b);
    b.rota[0] = 0;
    b.visitado[0] = 1;
    double limiteRaiz = reduzirMatriz(raiz, n);
    if (limiteRaiz < b.melhorCusto)
        explorar(&b, raiz, limiteRaiz, 0.0, 0, 1);

    free(raiz);
    free(b.visitado);
    free(b.rota);
    return b.erroMemoria ? INFINITY : b.melhorCusto;
}