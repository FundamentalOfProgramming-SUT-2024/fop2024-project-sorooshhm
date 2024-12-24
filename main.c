#include <ncurses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "menu.h"
#include "auth.h"

int add(int, int);
int main()
{
    initscr();
    noecho();
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    // check authorization for auth menu
    if (!isAuthorized())
    {
        WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);

        char **menu = createAuthMenu();
        wattron(menuWin, A_BLINK);
        mvwprintw(menuWin, 1, 25, "Welcom to the Rogue world !!");
        wrefresh(menuWin);
        wattroff(menuWin, A_BLINK);

        mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------------");
        wmove(menuWin, 3, 25);

        wrefresh(menuWin);
        int highlight = handleMenuSelection(menuWin, menu, 3);
        printw("You choose %s", menu[highlight]);
        refresh();
    }
    // create pre-start menu
    else
    {
        WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);
        char **menu = createPreStartMenu();
        wattron(menuWin, A_BLINK);
        mvwprintw(menuWin, 1, 25, "Choos one of the options");
        wrefresh(menuWin);
        wattroff(menuWin, A_BLINK);

        mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------------");
        wmove(menuWin, 3, 25);

        wrefresh(menuWin);
        int highlight = handleMenuSelection(menuWin, menu, 2);
        printw("You choose %s", menu[highlight]);
        refresh();
    }

    getchar();
    endwin();
    return 0;
}