#include <ncurses.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include "auth.h"

typedef struct
{
    int x;
    int y;
} Door;

typedef struct
{
    int startX;
    int startY;
    int height;
    int width;
    Door **doors;
} Room;

typedef struct
{
    int x;
    int y;
    int health;
} Player;

extern int maxY, maxX;

void createRoom(Room *room, int rangeX, int rangeY)
{
    int width = (rand() % 4) + 4;
    int height = (rand() % 4) + 4;
    int x = (rand() % 48) + rangeX - width;
    int y = (rand() % 17) + rangeY - height;
    if (x < 0)
    {
        x = 0;
    }
    if (x > 144)
    {
        x = 85;
    }
    if (y < 4)
    {
        y = 5;
    }
    if (y > 40)
    {
        y = 26;
    }
    room->startX = x;
    room->startY = y;
    room->height = height;
    room->width = width;
}

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    printf("%d , %d\n", maxX, maxY);

    Room **rooms = (Room **)malloc(6 * sizeof(Room *));
    int rangeX = 0, rangeY = 4;
    for (int i = 0; i < 6; i++)
    {
        rooms[i] = (Room *)malloc(sizeof(Room));
        createRoom(rooms[i], (i % 3) * 48 + rangeX, (i % 2) * 17 + rangeY);
        int x, y, width, height;
        x = rooms[i]->startX;
        y = rooms[i]->startY;
        height = rooms[i]->height;
        width = rooms[i]->width;
        // printf("x %d y %d height %d width %d\n", x, y, height, width);

        for (int j = x; j < x + width; j++)
        {
            mvprintw(y, j, "_");
            mvprintw(y + height - 1, j, "_");
        }
        for (int j = y + 1; j < y + height; j++)
        {
            mvprintw(j, x - 1, "|");
            mvprintw(j, x + width, "|");
            for (int k = x + 1; k < x + width - 1; k++)
            {
                if(j==y+height-1){
                    continue;
                }
                mvprintw(j, k, ".");
            }
        }
        refresh();
    }
    while (1)
    {
        char c = getchar();
        if (c == 27)
        {
            Mix_FreeMusic(music);
            break;
        }
    }
    return;
};
