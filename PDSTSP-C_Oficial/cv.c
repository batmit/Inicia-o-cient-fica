#include <stdio.h>
#include "cv.h"
#include "main.h"
#include<math.h>
#include<stdbool.h>
#include <stdlib.h>

void construirMatrizCaminhao(Instancia *inst){

    alocarMatrizCaminhao(inst);
    //No PDSTSP-C o caminhão utiliza distância manhathan
    //A distância dos drones é sim euclidiana
    for (int i = 0; i < inst->n; i++) {

        for (int j = 0; j < inst->n; j++) {

            double dx = fabs(inst->clientes[i].x -
                             inst->clientes[j].x);

            double dy = fabs(inst->clientes[i].y -
                             inst->clientes[j].y);

            inst->tempoCaminhao[i][j] = (dx + dy)/ inst->truckSpeed * 60.0;
        }
    }


}


int alocarMatrizCaminhao(Instancia *inst)
{
    inst->tempoCaminhao = calloc(
        inst->n, sizeof(double *)
    );

    if (inst->tempoCaminhao == NULL)
        return 0;

    for (int i = 0; i < inst->n; i++)
    {
        inst->tempoCaminhao[i] = malloc(
            inst->n * sizeof(double)
        );

        if (inst->tempoCaminhao[i] == NULL)
            return 0;
    }

    return 1;
}

void alocarTempoDrone(Instancia *inst)
{
    // Aloca as linhas
    inst->tempoDrone = malloc(inst->n * sizeof(double *));

    if (inst->tempoDrone == NULL) {
        printf("Erro ao alocar matriz tempoDrone.\n");
        return;
    }

    // Aloca as colunas de cada linha
    for (int i = 0; i < inst->n; i++) {

        inst->tempoDrone[i] = malloc(
            (inst->numDrones + 1) * sizeof(double)
        );

        if (inst->tempoDrone[i] == NULL) {
            printf("Erro ao alocar linha %d.\n", i);

            // Libera as linhas ja alocadas
            for (int j = 0; j < i; j++) {
                free(inst->tempoDrone[j]);
            }

            free(inst->tempoDrone);
            inst->tempoDrone = NULL;
            return;
        }

        // Inicializa os tempos como inviaveis
        for (int k = 0; k <= inst->numDrones; k++) {
            inst->tempoDrone[i][k] = INFINITY;
        }
    }
}

//BREACH AND BOUND

