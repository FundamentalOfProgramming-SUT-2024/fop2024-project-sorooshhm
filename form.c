#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "menu.h"

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
        mvwprintw(win, y + 4 * state + 1, x + 2, headers[state]);
        WINDOW *input = creaetMenuWindow(3, 35, parentY + y + 4 * state + 2, parentX + x + 1);
        curs_set(true);
        wrefresh(win);
        move(parentY + y + 4 * state + 3, parentX + x + 2);
        char s[10];
        getstr(s);
        result[state] = s;
    }

    return result;
}