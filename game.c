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
    int width = (rand() % 6) + 6;
    int height = (rand() % 6) + 4;
    int x = (rand() % 49) + rangeX - width;
    int y = (rand() % 18) + rangeY - height;
    if (x <= 0)
    {
        x = 2;
    }
    if (x + width >= 144)
    {
        x = 85;
    }
    if (y <= 4)
    {
        y = 6;
    }
    if (y + height >= 40)
    {
        y = 26;
    }
    room->doors = (Door**)malloc(2*sizeof(Door*));
    for (int i = 0; i < 2; i++)
    {
        room->doors[i] = (Door*)malloc(sizeof(Door));
    }
    
    room->doors[0]->x = rand()%(width-1) + x;
    room->doors[1]->x = rand()%(width-1) + x;
    room->doors[0]->y = y;
    room->doors[1]->y = y + height -1;
    
    room->startX = x;
    room->startY = y;
    room->height = height;
    room->width = width;
}

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    printf("%d , %d\n", maxX, maxY);
    noecho();

    Room **rooms = (Room **)malloc(6 * sizeof(Room *));
    int rangeX = 3, rangeY = 4;
    for (int i = 0; i < 6; i++)
    {
        rooms[i] = (Room *)malloc(sizeof(Room));
        createRoom(rooms[i], (i % 3) * 49 + rangeX, (i % 2) * 18 + rangeY);
        int x, y, width, height;
        x = rooms[i]->startX;
        y = rooms[i]->startY;
        height = rooms[i]->height;
        width = rooms[i]->width;
        // printw("x %d y %d height %d width %d ", x, y, height, width);

        for (int j = x; j < x + width; j++)
        {
            mvprintw(y, j, "--");
            mvprintw(y + height - 1, j, "--");
            refresh();
        }
        for (int j = y + 1; j < y + height; j++)
        {
            mvprintw(j, x - 1, "|");
            mvprintw(j, x + width, "|");
            refresh();

            for (int k = x + 1; k < x + width - 1; k++)
            {
                if (j == y + height - 1)
                {
                    continue;
                }
                mvprintw(j, k, ":");
            }
            refresh();
        }
        for (int j = 0; j < 2; j++)
        {
            mvprintw(rooms[i]->doors[j]->y,rooms[i]->doors[j]->x , "+");
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
