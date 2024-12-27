#include <ncurses.h>
#include <stdlib.h>

char **handleInput(WINDOW *win, int count, char **headers, int, int);
char **createHedares(char type)
{
    if (type == 'L')
    {
        char **headers = (char **)malloc(2 * sizeof(char *));
        headers[0] = "Username";
        headers[1] = "Password";
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