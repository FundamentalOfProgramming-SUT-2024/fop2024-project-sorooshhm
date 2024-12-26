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
    }

    return result;
}