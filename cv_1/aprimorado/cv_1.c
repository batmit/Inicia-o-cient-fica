#include<stdio.h>
#include <stdlib.h>
#include "cv_1.h"
#include<math.h>
#include<float.h>
#include <ncurses.h>

//Usarei a matriz das distâncias
//Será circular
//Latitude e longitude double

struct cidade{

    double latitude, longitude;

};



int main(){

    Cidade *vetorCidades = malloc(CIDADES* sizeof(Cidade));
    int *caminho = malloc(CIDADES * sizeof(int));
    int *resultadoFinal = malloc(CIDADES * sizeof(int));

    //Apenas lerei os valores e colocarei o identificador em cada Cidade
    for(int i = 0; i < CIDADES; i++){
        resultadoFinal[i] = 0;
        caminho[i] = -1;
        scanf("%lf", &vetorCidades[i].latitude);
        scanf("%lf", &vetorCidades[i].longitude);
    }

    double **matrizDistancia = malloc(CIDADES * sizeof(double*));
    for(int i = 0; i < CIDADES; i++){

        matrizDistancia[i] = malloc(CIDADES * sizeof(double));
        for(int j = 0; j < CIDADES; j++){

            matrizDistancia[i][j] = distanciaEuclidiana(vetorCidades[i].latitude, vetorCidades[j].latitude, vetorCidades[i].longitude, vetorCidades[j].longitude);


        }
    }

    //terei que fixar a primeira posição da cidade, por questões de complexidad
    caminho[0] = 0;
    double n = DBL_MAX;
    int posicao = 1;
    cacheiroViajante(caminho, &n, posicao, resultadoFinal, 0, matrizDistancia);

    printf("Menor distancia encontrada: %.6lf\n", n);
    printf("Caminho: ");
    for (int i = 0; i < CIDADES; i++) {
        printf("%d ", resultadoFinal[i]);
    }
    printf("\n");

    mostrarRotaNcurses(vetorCidades, resultadoFinal, CIDADES);

    //printf("Menor distancia encontrada: %lf\nCaminho: ", n);
    //for(int i = 0; i < CIDADES; i++){

    //    printf("%d, ", resultadoFinal[i]);

    //}




    free(resultadoFinal);
    free(caminho);
    free(vetorCidades);
    for(int i = 0; i < CIDADES; i++){

        free(matrizDistancia[i]);

    }
    free(matrizDistancia);
    return 0;

}

void cacheiroViajante(int *caminho, double *n, int pos, int *resultadoFinal, double soma, double **matrizDistancia){

    if(pos == CIDADES){

        double total = soma +  matrizDistancia[caminho[CIDADES - 1]][caminho[0]];
        if(total < *n){

            *n = total;

            //Copia para o resultado final o caminho
            for(int i = 0; i < CIDADES; i++){

                resultadoFinal[i] = caminho[i];

            }

        }
        return;
    }


    for(int i = 1; i < CIDADES; i++){
        int notParticipate = 0;
        for(int j = 0; j < pos; j++){

            if(caminho[j] == i){
                notParticipate = 1;
            }


        }

        //Se a cidade já não está no caminho
        if(notParticipate == 0){
            caminho[pos] = i;
            cacheiroViajante(caminho, n, pos + 1, resultadoFinal, soma + matrizDistancia[caminho[pos-1]][i], matrizDistancia);

            caminho[pos] = -1;

        }




    }



}

//Não estou considerando a distãncia da volta
double distanciaEuclidiana(double x1, double y1, double x2, double y2){

    double  resposta;


    resposta = (pow((x1 - y1), 2)) + (pow((x2 - y2), 2));
    resposta = sqrt(resposta);



    return resposta;
}


//Fato importante e interessante: em somas sucessivas nas chamadas recurssivas, você deve alterar o valor na chamada da função,
//caso altere antes, esse valor fica alterado em toda a camada da recurssão, o que não é o certo



void mostrarRotaNcurses(Cidade *cidades, int *rota, int n) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    int altura, largura;
    getmaxyx(stdscr, altura, largura);

    double minLat, maxLat, minLon, maxLon;
    limitesMapa(cidades, n, &minLat, &maxLat, &minLon, &maxLon);

    PontoTela *pts = malloc(n * sizeof(PontoTela));
    for (int i = 0; i < n; i++) {
        pts[i] = mapearCidadeTela(cidades[i], minLat, maxLat, minLon, maxLon,
                                  largura, altura, 2);
    }

    clear();
    box(stdscr, 0, 0);
    desenharRota(pts, rota, n, 1);
    desenharCidades(cidades, pts, n);
    destacarRota(pts, rota, n);

    mvprintw(1, 2, "Rota calculada");
    mvprintw(2, 2, "Inicio: cidade %d", rota[0]);
    mvprintw(3, 2, "Ultima antes de voltar: cidade %d", rota[n - 1]);
    mvprintw(altura - 2, 2, "Pressione qualquer tecla para sair");
    refresh();
    getch();
    endwin();

    free(pts);
}

void limitesMapa(Cidade *cidades, int n,
                 double *minLat, double *maxLat,
                 double *minLon, double *maxLon) {
    *minLat = *maxLat = cidades[0].latitude;
    *minLon = *maxLon = cidades[0].longitude;

    for (int i = 1; i < n; i++) {
        if (cidades[i].latitude < *minLat) *minLat = cidades[i].latitude;
        if (cidades[i].latitude > *maxLat) *maxLat = cidades[i].latitude;
        if (cidades[i].longitude < *minLon) *minLon = cidades[i].longitude;
        if (cidades[i].longitude > *maxLon) *maxLon = cidades[i].longitude;
    }
}

PontoTela mapearCidadeTela(Cidade cidade,
                           double minLat, double maxLat,
                           double minLon, double maxLon,
                           int largura, int altura,
                           int margem) {
    PontoTela p;

    double faixaLon = maxLon - minLon;
    double faixaLat = maxLat - minLat;

    if (faixaLon == 0) faixaLon = 1;
    if (faixaLat == 0) faixaLat = 1;

    int areaLargura = largura - 2 * margem;
    int areaAltura  = altura  - 2 * margem;

    p.x = margem + (int)(((cidade.longitude - minLon) / faixaLon) * (areaLargura - 1));
    p.y = margem + (int)(((maxLat - cidade.latitude) / faixaLat) * (areaAltura - 1));

    if (p.x < 0) p.x = 0;
    if (p.x >= largura) p.x = largura - 1;
    if (p.y < 0) p.y = 0;
    if (p.y >= altura) p.y = altura - 1;

    return p;
}

void desenharCidades(Cidade *cidades, PontoTela *pts, int n) {
    for (int i = 0; i < n; i++) {
        mvaddch(pts[i].y, pts[i].x, 'o');
        mvprintw(pts[i].y, pts[i].x + 1, "%d", i);
    }
}


void desenharRota(PontoTela *pts, int *rota, int n, int fecharCiclo) {
    for (int i = 0; i < n - 1; i++) {
        int a = rota[i];
        int b = rota[i + 1];
        desenharLinhaAnimada(pts[a].x, pts[a].y, pts[b].x, pts[b].y, '.', 20);
    }

    if (fecharCiclo && n > 1) {
        int a = rota[n - 1];
        int b = rota[0];
        desenharLinhaAnimada(pts[a].x, pts[a].y, pts[b].x, pts[b].y, '.', 20);
    }
}


void destacarRota(PontoTela *pts, int *rota, int n) {
    for (int i = 0; i < n; i++) {
        int idx = rota[i];
        attron(A_BOLD);
        if (i == 0) {
            mvaddch(pts[idx].y, pts[idx].x, 'I'); // início
        } else if (i == n - 1) {
            mvaddch(pts[idx].y, pts[idx].x, 'F'); // última antes de voltar
        } else {
            mvaddch(pts[idx].y, pts[idx].x, 'O');
        }

        attroff(A_BOLD);
    }
}

void desenharLinha(int x0, int y0, int x1, int y1, chtype ch) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        mvaddch(y0, x0, ch);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}


void animarRota(PontoTela *pts, int *rota, int n, int fecharCiclo) {
    for (int i = 0; i < n - 1; i++) {
        int a = rota[i];
        int b = rota[i + 1];

        desenharLinha(pts[a].x, pts[a].y, pts[b].x, pts[b].y, '.');
        refresh();
        napms(300);
    }

    if (fecharCiclo && n > 1) {
        int a = rota[n - 1];
        int b = rota[0];
        desenharLinha(pts[a].x, pts[a].y, pts[b].x, pts[b].y, '.');
        refresh();
        napms(300);
    }
}


void desenharLinhaAnimada(int x0, int y0, int x1, int y1, chtype ch, int atraso) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        mvaddch(y0, x0, ch);
        refresh();
        napms(atraso);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}