#include <ncurses.h>

WINDOW *creaetMenuWindow(int height, int width, int y, int x)
{
    WINDOW *win = newwin(height, width, y, x);
    refresh();
    box(win, 0, 0);
    wrefresh(win);
    return win;
}

int handleMenuSelection(WINDOW *win, char menu[][10], int size)
{
    keypad(win, true);
    int highlight = 0;
    while (1)
    {
        for (int i = 0; i < 3; i++)
        {
            if (i == highlight)
            {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, i + 1, 2, menu[i]);
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