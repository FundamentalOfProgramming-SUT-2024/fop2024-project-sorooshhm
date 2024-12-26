#include <ncurses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "menu.h"
#include "form.h"
#include "auth.h"

int main()
{
    initscr();
    // noecho();
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    // check authorization for auth menu
    char checkAuth = isAuthorized();
    if (checkAuth == '0')
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
        char choice = menu[highlight][0];
        printw("You choose %s", menu[highlight]);
        refresh();
        getchar();
        clear();
        refresh();
        // clear();
        refresh();

        if (choice == 'L')
        {
            while (1)
            {

                WINDOW *formWin = creaetMenuWindow(20, maxX / 4, maxY / 2 - 15, maxX / 2 - 15);
                char **headers = createHedares(choice);
                wmove(formWin, 3, maxX / 2 - 15);

                char **result = handleInput(formWin, 2, headers, maxY / 2 - 15, maxX / 2 - 15);
                // wclear(formWin);
                // wrefresh(formWin);

                char **message = malloc(sizeof(char *));
                User *user = login(result[0], result[1], message);
                if (user == NULL)
                {
                    mvwprintw(formWin, 10, 4, *message);
                    mvwprintw(formWin, 12, 4, "Press any key to continue ...");
                    wrefresh(formWin);
                    getchar();
                    clear();
                }
                else
                {
                    break;
                }
            }
        }
    }
    // create pre-start menu
    else if (checkAuth == '1')
    {
        WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);
        char **menu = createPreStartMenu(checkAuth);
        wattron(menuWin, A_BLINK);
        mvwprintw(menuWin, 1, 25, "Choos one of the options");
        wrefresh(menuWin);
        wattroff(menuWin, A_BLINK);

        mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------------");
        wmove(menuWin, 3, 25);

        wrefresh(menuWin);
        int highlight = handleMenuSelection(menuWin, menu, checkAuth == '1' ? 2 : 1);
        printw("You choose %s", menu[highlight]);
        refresh();
    }

    getchar();
    endwin();
    return 0;
}