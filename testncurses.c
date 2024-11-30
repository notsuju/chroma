#include <ncurses.h>

int main()
{
    initscr();
    printw("Hellow");
    mvprintw(1, 10, "World");
    refresh();
    getch();
    endwin();
}
