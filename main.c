#include <ncurses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>
#include "menu.h"
#include "form.h"
#include "auth.h"

User *user;
int maxY, maxX;

void preStartMenu(char type);

int main()
{

    initscr();
    // noecho();
    user = malloc(sizeof(User));
    cbreak();
    getmaxyx(stdscr, maxY, maxX);
    // check authorization for auth menu
    char checkAuth = isAuthorized(&user);
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
        wclear(menuWin);
        free(menu);
        clear();
        refresh();
        // clear();

        if (choice == 'L')
        {
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
                    mvwprintw(formWin, 12, 4, "%s", *message);
                    mvwprintw(formWin, 14, 4, "Press any key to continue ...");
                    wrefresh(formWin);
                    getchar();
                    wclear(formWin);
                    free(headers);
                    clear();
                }
                else
                {
                    checkAuth = '1';
                    wclear(formWin);
                    free(headers);
                    clear();
                    refresh();
                    break;
                }
            }
        }
        else if (choice == 'R')
        {
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
                    wclear(formWin);
                    free(headers);
                    clear();
                }
                else
                {
                    checkAuth = '1';
                    wclear(formWin);
                    free(headers);
                    clear();
                    refresh();
                    break;
                }
            }
        }
        else if (choice == 'G')
        {

            WINDOW *formWin = creaetMenuWindow(20, maxX / 3 + 5, maxY / 2 - 15, maxX / 6 + 15);
            char **headers = createHedares(choice);
            wmove(formWin, 3, maxX / 2 - 15);
            char **result = handleInput(formWin, 1, headers, maxY / 2 - 15, maxX / 6 + 15);
            char **message = malloc(sizeof(char *));
            user->username = result[0];
            checkAuth = '2';
            wclear(formWin);
            free(headers);
            clear();
            refresh();
        }
    }
        // create pre-start menu
        preStartMenu(checkAuth);
    // getchar();
    endwin();
    return 0;
}

void preStartMenu(char type)
{
    while (1)
    {

        if (type == '1')
        {
            WINDOW *menuWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
            char **menu = createPreStartMenu(type);
            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Choos one of the options");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);

            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, 6);
            printw("You choose %s", menu[highlight]);
            refresh();
            getchar();
            wclear(menuWin);
            clear();
            free(menu);
            refresh();
            if (highlight == 0)
            {
            }
            else if (highlight == 1)
            {
            }
            else if (highlight == 2)
            {
            }
            else if (highlight == 3)
            {
            }
            else if (highlight == 4)
            {
                WINDOW *profileWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);
                wattron(profileWin, A_BLINK);
                mvwprintw(profileWin, 1, 25, "Profile");
                wrefresh(profileWin);
                wattroff(profileWin, A_BLINK);

                mvwprintw(profileWin, 2, 1, "----------------------------------------------------------------------");

                wrefresh(profileWin);
                mvwprintw(profileWin, 4, 20, "Username : %s", user->username);
                mvwprintw(profileWin, 6, 20, "Email : %s", user->email);
                mvwprintw(profileWin, 8, 20, "Games : %d", user->games);
                mvwprintw(profileWin, 10, 20, "Golds : %d", user->golds);
                mvwprintw(profileWin, 12, 20, "Score : %d", user->score);
                mvwprintw(profileWin, 14, 20, "Gameplay : %d", user->gameplay);
                wrefresh(profileWin);
                while (1)
                {
                    char c = getchar();
                    if (c == 27)
                    {
                        wclear(profileWin);
                        clear();
                        refresh();

                        break;
                    }
                }
            }
            else if (highlight == 5)
            {
                WINDOW *logoutWin = creaetMenuWindow(10, maxX / 3, maxY / 2 - 15, maxX / 4);
                mvwprintw(logoutWin, 1, 10, "Do you want to logout?");
                mvwprintw(logoutWin, 2, 1, "----------------------------------------------");
                wmove(logoutWin, 3, 10);

                char **options = (char **)malloc(2 * sizeof(char *));
                options[0] = "Yes";
                options[1] = "No";
                int highlight = handleMenuSelection(logoutWin, options, 2);
                if (highlight == 0)
                {
                    logout();
                    mvwprintw(logoutWin, 8, 5, "Goodbye :( ....");
                    wrefresh(logoutWin);
                    sleep(1);
                    free(options);
                    wclear(logoutWin);
                    endwin();
                    exit(0);
                    // endwin();
                }
                else if (highlight == 1)
                {
                    wclear(logoutWin);
                    clear();
                    refresh();
                }
            }
        }
        if (type == '2')
        {
            WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);
            char **menu = createPreStartMenu(type);
            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Choos one of the options");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);

            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, 1);
            printw("You choose %s", menu[highlight]);
            refresh();
        }
        usleep(500);
    }
}
