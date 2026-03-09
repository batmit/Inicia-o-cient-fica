#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>

#define CIDADES 8

typedef struct {
    double latitude;
    double longitude;
    char nome[20];
} Cidade;

//Defino a struct do ponto na tela, com coordenadas cartesianas
typedef struct {
    int x;
    int y;
} PontoTela;

/* ---------------------------------------------------
   Encontra mínimo e máximo para normalizar coordenadas
--------------------------------------------------- */
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

/* ---------------------------------------------------
   Converte latitude/longitude em x,y da tela
   A tela cresce para baixo, então invertimos y
--------------------------------------------------- */
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

/* ---------------------------------------------------
   Desenha uma linha entre dois pontos (Bresenham)
--------------------------------------------------- */
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

/* ---------------------------------------------------
   Desenha todas as cidades
--------------------------------------------------- */
void desenharCidades(Cidade *cidades, PontoTela *pts, int n) {
    for (int i = 0; i < n; i++) {
        mvaddch(pts[i].y, pts[i].x, 'o');
        mvprintw(pts[i].y, pts[i].x + 1, "%s", cidades[i].nome);
    }
}

/* ---------------------------------------------------
   Desenha uma rota: vetor com índices das cidades
   Ex.: rota = {0, 3, 2, 5, 1, 4, 6, 7}
--------------------------------------------------- */
void desenharRota(PontoTela *pts, int *rota, int n, int fecharCiclo) {
    for (int i = 0; i < n - 1; i++) {
        int a = rota[i];
        int b = rota[i + 1];
        desenharLinha(pts[a].x, pts[a].y, pts[b].x, pts[b].y, '.');
    }

    if (fecharCiclo && n > 1) {
        int a = rota[n - 1];
        int b = rota[0];
        desenharLinha(pts[a].x, pts[a].y, pts[b].x, pts[b].y, '.');
    }
}

/* ---------------------------------------------------
   Reforça visualmente os pontos da rota
--------------------------------------------------- */
void destacarRota(PontoTela *pts, int *rota, int n) {
    for (int i = 0; i < n; i++) {
        int idx = rota[i];
        attron(A_BOLD);
        mvaddch(pts[idx].y, pts[idx].x, 'O');
        attroff(A_BOLD);
    }
}

int main() {
    Cidade cidades[CIDADES] = {
        {-19.92, -43.94, "A"},
        {-19.85, -43.90, "B"},
        {-19.88, -43.99, "C"},
        {-19.95, -43.89, "D"},
        {-19.80, -43.95, "E"},
        {-19.90, -44.02, "F"},
        {-19.78, -43.87, "G"},
        {-19.97, -44.01, "H"}
    };

    /* Exemplo de rota já calculada pelo seu algoritmo */
    int rota[CIDADES] = {0, 2, 5, 7, 3, 1, 6, 4};

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    }

    int altura, largura;
    getmaxyx(stdscr, altura, largura);

    double minLat, maxLat, minLon, maxLon;
    limitesMapa(cidades, CIDADES, &minLat, &maxLat, &minLon, &maxLon);

    PontoTela pts[CIDADES];
    for (int i = 0; i < CIDADES; i++) {
        pts[i] = mapearCidadeTela(cidades[i],
                                  minLat, maxLat,
                                  minLon, maxLon,
                                  largura, altura,
                                  2);
    }

    clear();

    box(stdscr, 0, 0);

    if (has_colors()) attron(COLOR_PAIR(1));
    desenharRota(pts, rota, CIDADES, 1);
    if (has_colors()) attroff(COLOR_PAIR(1));

    if (has_colors()) attron(COLOR_PAIR(2));
    desenharCidades(cidades, pts, CIDADES);
    destacarRota(pts, rota, CIDADES);
    if (has_colors()) attroff(COLOR_PAIR(2));

    mvprintw(1, 2, "Visualizacao da rota mais rapida");
    mvprintw(altura - 2, 2, "Pressione qualquer tecla para sair");

    refresh();
    getch();
    endwin();

    return 0;
}