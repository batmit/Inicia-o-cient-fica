#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tspd.h"

static void inicializarInstancia(InstanciaTSPD *inst) {
    inst->cidades = NULL;
    inst->podeDrone = NULL;
    inst->n = 0;
    inst->fatorCaminhao = 0.0;
    inst->fatorDrone = 0.0;
    inst->maxFly = -1.0;
}

static void liberarInstancia(InstanciaTSPD *inst) {
    free(inst->cidades);
    free(inst->podeDrone);

    inst->cidades = NULL;
    inst->podeDrone = NULL;
    inst->n = 0;
}



static int lerTokenIgnorandoComentarios(FILE *arquivo, char *token, int tamanho) {
    int c;

    while ((c = fgetc(arquivo)) != EOF) {

        if (c == '/') {
            int prox = fgetc(arquivo);

            if (prox == '*') {
                int anterior = 0;
                int atual;

                while ((atual = fgetc(arquivo)) != EOF) {
                    if (anterior == '*' && atual == '/') {
                        break;
                    }

                    anterior = atual;
                }

                continue;
            } else {
                ungetc(prox, arquivo);
                ungetc(c, arquivo);
            }
        }

        if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
            continue;
        }

        ungetc(c, arquivo);

        if (fscanf(arquivo, "%63s", token) == 1) {
            return 1;
        }
    }

    return 0;
}

int lerInstanciaTSPD(const char *nomeArquivo, InstanciaTSPD *inst) {
    FILE *arquivo = fopen(nomeArquivo, "r");

    if (arquivo == NULL) {
        printf("Erro: não foi possível abrir o arquivo %s\n", nomeArquivo);
        return 0;
    }

    inicializarInstancia(inst);

    char token[64];

    if (!lerTokenIgnorandoComentarios(arquivo, token, sizeof(token))) {
        printf("Erro ao ler fator do caminhão.\n");
        fclose(arquivo);
        return 0;
    }
    inst->fatorCaminhao = atof(token);

    if (!lerTokenIgnorandoComentarios(arquivo, token, sizeof(token))) {
        printf("Erro ao ler fator do drone.\n");
        fclose(arquivo);
        return 0;
    }
    inst->fatorDrone = atof(token);

    if (!lerTokenIgnorandoComentarios(arquivo, token, sizeof(token))) {
        printf("Erro ao ler número de nós.\n");
        fclose(arquivo);
        return 0;
    }
    inst->n = atoi(token);

    inst->cidades = malloc(inst->n * sizeof(Cidade));
    inst->podeDrone = malloc(inst->n * sizeof(int));

    if (inst->cidades == NULL || inst->podeDrone == NULL) {
        printf("Erro: falha na alocação de memória.\n");
        liberarInstancia(inst);
        fclose(arquivo);
        return 0;
    }

    for (int i = 0; i < inst->n; i++) {

        if (!lerTokenIgnorandoComentarios(arquivo, token, sizeof(token))) {
            printf("Erro ao ler latitude da cidade %d.\n", i);
            liberarInstancia(inst);
            fclose(arquivo);
            return 0;
        }
        inst->cidades[i].latitude = atof(token);

        if (!lerTokenIgnorandoComentarios(arquivo, token, sizeof(token))) {
            printf("Erro ao ler longitude da cidade %d.\n", i);
            liberarInstancia(inst);
            fclose(arquivo);
            return 0;
        }
        inst->cidades[i].longitude = atof(token);

        if (!lerTokenIgnorandoComentarios(arquivo, token, sizeof(token))) {
            printf("Erro ao ler identificador da cidade %d.\n", i);
            liberarInstancia(inst);
            fclose(arquivo);
            return 0;
        }

        if (i == 0) {
            inst->podeDrone[i] = 0; // depósito nunca é atendido por drone
        }
        else if (token[0] == 'v' || token[0] == 'V') {
            inst->podeDrone[i] = 1; // v = pode drone
        }
        else if (token[0] == 'u' || token[0] == 'U') {
            inst->podeDrone[i] = 0; // u = não pode drone
        }
        else {
            inst->podeDrone[i] = 0; // segurança
        }
    }

    fclose(arquivo);
    return 1;
}