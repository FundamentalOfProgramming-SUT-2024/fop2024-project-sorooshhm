#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "menu.h"
#include "utils.h"

char **handleInput(WINDOW *win, int count, char **headers, int parentY, int parentX)
{
    int x, y;
    x = getcurx(win);
    y = getcury(win);
    y++;
    char **result = (char **)malloc(count * sizeof(char *));
    for (int i = 0; i < count; i++)
    {
        result[i] = (char *)malloc(100 * sizeof(char));
    }

    int state = 0;
    for (; state < count; state++)
    {
        mvwprintw(win, y + 5 * state + 1, x + 2, "%s", headers[state]);
        WINDOW *input = creaetMenuWindow(3, 30, parentY + y + 5 * state + 2, parentX + x + 1);
        curs_set(true);
        wrefresh(win);
        move(parentY + y + 5 * state + 3, parentX + x + 2);
        getstr(result[state]);
        if (strlen(result[state]) == 0)
        {
            move(parentY + y + 5 * state + 5, parentX + x + 2);
            printw("Input can not be empty");
            refresh();
            getchar();
            move(parentY + y + 5 * state + 5, parentX + x + 2);
            printw("                                              ");
            state--;
            continue;
        }
        if (count == 3)
        {
            if (state == 1)
            {
                int check = validateEmail(result[1]);
                if (!check)
                {
                    move(parentY + y + 5 * state + 5, parentX + x + 2);
                    printw("Email is not valid !! press key to input again");
                    refresh();
                    getchar();
                    move(parentY + y + 5 * state + 5, parentX + x + 2);
                    printw("                                              ");
                    state--;
                }
            }
            else if (state == 2)
            {
                if (strcmp(result[2], "RANDOM") == 0)
                {
                    generateRadomPass(result[2]);
                    mvprintw(parentY + y + 5 * state + 3, parentX + x + 2, "%s", result[2]);
                    refresh();
                }
                else
                {
                    int check = validatePassword(result[2]);
                    if (!check)
                    {
                        move(parentY + y + 5 * state + 5, parentX + x + 2);
                        printw("Password is not valid !! press key to input again");
                        refresh();
                        getchar();
                        move(parentY + y + 5 * state + 5, parentX + x + 2);
                        printw("                                                 ");
                        state--;
                    }
                }
            }
        }
        else if (count == 2)
        {
            if (state == 1)
            {
                if (strcmp(result[1], "FORGOT") == 0)
                {
                    mvprintw(parentY + y + 5 * state + 5, parentX + x + 2, "Enter the cheetcode");
                    refresh();
                    noecho();
                    getstr(result[1]);
                }
            }
        }
    }

    return result;
}
char **createHedares(char type)
{
    if (type == 'L')
    {
        char **headers = (char **)malloc(2 * sizeof(char *));
        headers[0] = "Username";
        headers[1] = "Password (FORGOT for forgot)";
        return headers;
    }
    if (type == 'R')
    {
        char **headers = (char **)malloc(3 * sizeof(char *));
        headers[0] = "Username";
        headers[1] = "Email";
        headers[2] = "Password (type RANDOM to use random password)";
        return headers;
    }
    if (type == 'G')
    {
        char **headers = (char **)malloc(3 * sizeof(char *));
        headers[0] = "Username";
        return headers;
    }
}