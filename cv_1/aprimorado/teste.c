#include <ncurses.h>

int main() {
    initscr();
    printw("Ncurses funcionando!");
    refresh();
    getch();
    endwin();
    return 0;
}