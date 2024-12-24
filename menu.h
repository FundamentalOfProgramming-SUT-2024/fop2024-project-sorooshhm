#include <ncurses.h>

WINDOW *creaetMenuWindow(int height, int width, int y, int x);
int handleMenuSelection(WINDOW *win, char menu[][10], int size);

