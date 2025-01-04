// #ifndef HEADER_H_FORM
// #define HEADER_H_FORM
#include <ncurses.h>
#include <stdlib.h>

char **handleInput(WINDOW *win, int count, char **headers, int, int);
char **createHedares(char type);
// #endif