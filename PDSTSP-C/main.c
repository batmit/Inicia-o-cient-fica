#include "cv.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    InstanciaPDSTSPC instancia;
    Solucao solucao;
    char caminho[4096], erro[512];
    const char *arquivo = NULL, *params = NULL;
    int i, somente_preprocessar = 0, resultado = 1;
    inicializar_instancia(&instancia);
    inicializar_solucao(&solucao);
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--preprocessar")) somente_preprocessar = 1;
        else if (!strcmp(argv[i], "--parametros") && i+1 < argc) params = argv[++i];
        else if (!strcmp(argv[i], "--help")) {
            puts("Uso: pdstspc.exe instancia.txt [--parametros other-params.txt] [--preprocessar]");
            return 0;
        } else if (argv[i][0] == '-' || arquivo) {
            fprintf(stderr,"Argumento invalido: %s (use --help)\n",argv[i]);
            return 1;
        } else arquivo = argv[i];
    }
    if (!arquivo) {
        printf("Caminho do arquivo PDSTSP-C: ");
        if (!fgets(caminho,sizeof caminho,stdin)) return 1;
        caminho[strcspn(caminho,"\r\n")] = '\0';
        arquivo = caminho;
    }
    if (!ler_instancia_pdstspc(arquivo,params,&instancia,erro,sizeof erro) ||
        !preprocessar_instancia(&instancia,erro,sizeof erro)) {
        fprintf(stderr,"Erro: %s\n",erro);
        goto fim;
    }
    imprimir_instancia(&instancia);
    if (!somente_preprocessar) {
        if (!heuristica(&instancia,&solucao,erro,sizeof erro)) {
            fprintf(stderr,"Erro: %s\n",erro);
            goto fim;
        }
        imprimir_solucao(&solucao);
    }
    resultado = 0;
fim:
    liberar_solucao(&solucao);
    liberar_instancia(&instancia);
    return resultado;
}
