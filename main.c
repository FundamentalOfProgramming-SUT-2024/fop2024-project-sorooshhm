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
    cbreak();
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

        mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
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
            User *user = malloc(sizeof(User));
            while (1)
            {

                WINDOW *formWin = creaetMenuWindow(20, maxX / 4, maxY / 2 - 15, maxX / 2 - 15);
                char **headers = createHedares(choice);
                wmove(formWin, 3, maxX / 2 - 15);

                char **result = handleInput(formWin, 2, headers, maxY / 2 - 15, maxX / 2 - 15);
                char **message = malloc(sizeof(char *));
                user = login(result[0], result[1], message);
                if (user == NULL)
                {
                    mvwprintw(formWin, 10, 4, "%s", *message);
                    mvwprintw(formWin, 12, 4, "Press any key to continue ...");
                    wrefresh(formWin);
                    getchar();
                    clear();
                }
                else
                {
                    checkAuth = '1';
                    clear();
                    refresh();
                    break;
                }
            }
        }
        else if (choice == 'R')
        {
            User *user = malloc(sizeof(User));
            while (1)
            {

                WINDOW *formWin = creaetMenuWindow(30, maxX / 3 + 5, maxY / 2 - 15, maxX / 6 + 15);
                char **headers = createHedares(choice);
                wmove(formWin, 3, maxX / 2 - 15);
                char **result = handleInput(formWin, 3, headers, maxY / 2 - 15, maxX / 6 + 15);
                char **message = malloc(sizeof(char *));
                user = registerUser(result[0], result[2], result[1], message);
                if (user == NULL)
                {
                    mvwprintw(formWin, 25, 4, "%s", *message);
                    mvwprintw(formWin, 27, 4, "Press any key to continue ...");
                    wrefresh(formWin);
                    getchar();
                    clear();
                }
                else
                {
                    checkAuth = '1';
                    clear();
                    refresh();
                    break;
                }
            }
        }
    }
    // create pre-start menu
    if (checkAuth == '1')
    {
        WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);
        char **menu = createPreStartMenu(checkAuth);
        wattron(menuWin, A_BLINK);
        mvwprintw(menuWin, 1, 25, "Choos one of the options");
        wrefresh(menuWin);
        wattroff(menuWin, A_BLINK);

        mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
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