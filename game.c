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
    int width = (rand() % 8) + 6;
    int height = (rand() % 6) + 4;
    int x = (rand() % 41) + rangeX;
    int y = (rand() % 18) + rangeY;
    if (x <= 0)
    {
        x = 2;
    }
    if (x + width >= rangeX + 41)
    {
        x = rangeX + 41 - width;
    }
    if (x > 130)
    {
        x = 120;
    }
    if (y <= 4)
    {
        y = 6;
    }
    if (y + height >= rangeY + 17)
    {
        y = rangeY + 17 - height;
    }
    room->doors = (Door **)malloc(2 * sizeof(Door *));
    for (int i = 0; i < 2; i++)
    {
        room->doors[i] = (Door *)malloc(sizeof(Door));
    }

    room->doors[0]->x = rand() % (width - 1) + x;
    room->doors[1]->x = rand() % (width - 1) + x;
    room->doors[0]->y = y;
    room->doors[1]->y = y + height;

    room->startX = x;
    room->startY = y;
    room->height = height;
    room->width = width;
}

void movePlayer(Player *player, int x, int y)
{
    mvprintw(player->y, player->x, ".");
    player->x = x;
    player->y = y;
    mvprintw(player->y, player->x, "@");
    refresh();
}

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    printf("%d , %d\n", maxX, maxY);
    noecho();
    Player player;
    Room **rooms = (Room **)malloc(6 * sizeof(Room *));
    int rangeX = 7, rangeY = 4;
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
            mvprintw(y, j, "-");
            mvprintw(y + height, j, "-");
            refresh();
        }
        for (int j = y + 1; j < y + height; j++)
        {
            mvprintw(j, x - 1, "|");
            mvprintw(j, x + width, "|");
            refresh();

            for (int k = x; k < x + width; k++)
            {
                mvprintw(j, k, ".");
            }
            refresh();
        }
        for (int j = 0; j < 2; j++)
        {
            if (i == 0 && j == 0)
            {
                continue;
            }
            if(i == 5 && j == 1) continue;
            mvprintw(rooms[i]->doors[j]->y, rooms[i]->doors[j]->x, "+");
        }
        refresh();
    }
    player.x = rooms[0]->startX + 1;
    player.y = rooms[0]->startY + 1;
    mvprintw(player.y, player.x, "@");
    refresh();
    keypad(stdscr, true);
    while (1)
    {
        char c = getchar();
        if (c == 27)
        {
            Mix_FreeMusic(music);
            break;
        }
        else if (c == 'w' || c == '8')
        {
            movePlayer(&player, player.x, player.y - 1);
        }
        else if (c == 's' || c == '2')
        {
            movePlayer(&player, player.x, player.y + 1);
        }
        else if (c == 'd' || c == '6')
        {
            movePlayer(&player, player.x + 1, player.y);
        }
        else if (c == 'a' || c == '4')
        {
            movePlayer(&player, player.x - 1, player.y);
        }
    }
    return;
};
