#include <ncurses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "menu.h"

int add(int, int);
int main()
{
    initscr();
    noecho();
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    WINDOW* menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);

    char menu[3][10] = {"Login", "Register", "Guest"};
    int highlight = handleMenuSelection(menuWin, menu, 3);
    printw("You choose %s", menu[highlight]);
    refresh();

    getchar();
    endwin();
    return 0;
}