#include <ncurses.h>

WINDOW *creaetMenuWindow(int height, int width, int y, int x);
int handleMenuSelection(WINDOW *win, char **menu, int size, int withEsc);
char **createAuthMenu();
char **createPreStartMenu(char check);
char **createSettingMenu();
