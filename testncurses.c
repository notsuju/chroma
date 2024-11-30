#include <ncurses.h>

int main()
{
    initscr();
    printw("Hellow");
    mvprintw(1, 10, "World");
    move(10,10);
    int x, y;
    getmaxyx(stdscr, x, y);
    printw("%d %d", x, y);
    start_color();
    init_pair(3, 5, 4);
    attrset(3);
    COLOR_PAIR(3);
    getch();
    endwin();
}
