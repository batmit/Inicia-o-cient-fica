#include <stdio.h>
#include <math.h>
#include "main.h"
#include "lerInstancias.h"
#include "cv.h"

int main(void)
{
    Instancia inst;



    lerInstancia("18-r-e.txt", &inst);

    lerTemposDrone("tau_kj_18-r-e.txt", &inst);


    liberarInstancia(&inst);

    return 0;
}
