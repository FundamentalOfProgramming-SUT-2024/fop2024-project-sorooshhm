#include <ncurses.h>
#include <stdlib.h>
#include "auth.h"

char **createAuthMenu()
{
    char **menu = (char **)malloc(3 * sizeof(char *));
    menu[0] = "Login";
    menu[1] = "Register";
    menu[2] = "Guest";
    return menu;
}

char **createPreStartMenu( char check)
{
    if (check == '1')
    {
        char **menu = (char **)malloc(5 * sizeof(char *));
        menu[0] = "Resume Last Game";
        menu[1] = "Start New Game";
        menu[2] = "Score Board";
        menu[3] = "Setting";
        menu[4] = "Profile";
        menu[5] = "Logout";

        return menu;
    }
    else {
        char **menu = (char **)malloc(1 * sizeof(char *));
        menu[0] = "Start New Game";
        return menu;
    }
}

WINDOW *creaetMenuWindow(int height, int width, int y, int x)
{
    WINDOW *win = newwin(height, width, y, x);
    curs_set(false);
    refresh();
    box(win, 0, 0);
    wrefresh(win);
    return win;
}

int handleMenuSelection(WINDOW *win, char **menu, int size)
{
    keypad(win, true);
    int highlight = 0;
    int x, y;
    x = getcurx(win);
    y = getcury(win);
    y++;
    while (1)
    {
        for (int i = 0; i < size; i++)
        {
            if (i == highlight)
            {
                wattron(win, A_REVERSE);
            }

            mvwprintw(win, y + i * 2, x + 1, "%d - %s", i + 1, menu[i]);
            wattroff(win, A_REVERSE);
        }
        int choice = wgetch(win);
        if (choice == KEY_UP)
        {
            highlight--;
            if (highlight < 0)
                highlight = 0;
        }
        else if (choice == KEY_DOWN)
        {
            highlight++;
            if (highlight > size - 1)
                highlight = size - 1;
        }
        else if (choice == 10)
        {
            break;
        }
    }
    return highlight;
}

