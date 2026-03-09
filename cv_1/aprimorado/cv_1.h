#ifndef CV_1_H
#define CV_1_H

#define CIDADES 12
#include <ncurses.h>

typedef struct cidade Cidade;
typedef struct {
    int x;
    int y;
} PontoTela;

void cacheiroViajante(int *caminho, double *n, int pos, int *resultadoFinal, double soma, double **matrizDistancia);
double distanciaEuclidiana(double x1, double y1, double x2, double y2);
void mostrarRotaNcurses(Cidade *cidades, int *rota, int n);
void limitesMapa(Cidade *cidades, int n,double *minLat, double *maxLat,double *minLon, double *maxLon);

PontoTela mapearCidadeTela(Cidade cidade,double minLat, double maxLat, double minLon, double maxLon,int largura, int altura,int margem);
void desenharLinha(int x0, int y0, int x1, int y1, chtype ch);

void desenharCidades(Cidade *cidades, PontoTela *pts, int n);
void desenharRota(PontoTela *pts, int *rota, int n, int fecharCiclo);
void destacarRota(PontoTela *pts, int *rota, int n);
void animarRota(PontoTela *pts, int *rota, int n, int fecharCiclo);
void desenharLinhaAnimada(int x0, int y0, int x1, int y1, chtype ch, int atraso);

#endif