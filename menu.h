#include <ncurses.h>

WINDOW *creaetMenuWindow(int height, int width, int y, int x);
int handleMenuSelection(WINDOW *win, char** menu, int size);
char **createAuthMenu();
char **createPreStartMenu();


