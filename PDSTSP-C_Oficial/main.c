#include <stdio.h>
#include <math.h>
#include "main.h"
#include "lerInstancias.h"
#include "cv.h"
#include "BaB.h"
#include <stdlib.h>
int main(void)
{
    Instancia inst;



    lerInstancia("18-r-e.txt", &inst);

    construirMatrizCaminhao(&inst);

    lerTemposDrone("tau_kj_18-r-e.txt", &inst);

    int *melhorRota = malloc(inst.n * sizeof(int));

    //Como passamos o tempo e não a distância, minimizaremos o tempo
    double melhorDistancia = branchAndBoundCaminhao(
        inst.tempoCaminhao,
        inst.n,
        melhorRota
    );

    printf("\nMelhor distancia: %.2lf\n",
           melhorDistancia);

    free(melhorRota);
    liberarInstancia(&inst);

    return 0;
}
