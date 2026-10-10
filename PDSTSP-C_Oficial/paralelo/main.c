#include <stdio.h>

#include "main.h"
#include "lerInstancias.h"

int main(void)
{
    Instancia inst;

    if (!lerInstancia("18-r-e.txt", &inst))
    {
        fprintf(stderr, "Nao foi possivel carregar a instancia.\n");
        return 1;
    }

    imprimirInstancia(&inst);

    liberarInstancia(&inst);

    return 0;
}
