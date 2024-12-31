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
    int type;
} Door;

typedef struct
{
    int startX;
    int startY;
    int height;
    int width;
    Door **doors;
    int doorCount;
    bool isVisible;
} Room;

typedef struct
{
    int x;
    int y;
    int health;
} Player;

extern int maxY, maxX;

void connectRooms(Door *door1, Door *door2)
{
    if (door1->type == 1 && door2->type == 1)
    {

        if ((door1->y < door2->y))
        {
            Door *tmp = (Door *)malloc(sizeof(Door));
            tmp->type = door1->type;
            tmp->x = door1->x;
            tmp->y = door1->y;
            door1 = door2;
            door2 = tmp;
        }
        int y = door1->y + (door1->type ? 1 : -1);
        mvprintw(y, door1->x, "#");
        int destX = door2->x;
        int x;
        for (x = door1->x; x != destX; x += (door1->x < door2->x) ? 1 : -1)
        {
            mvprintw(y, x, "#");
        }
        for (; y != door2->y; y += (door1->y < door2->y) ? 1 : -1)
        {
            mvprintw(y, x, "#");
        }
        mvprintw(door2->y, door2->x, "+");
    }
    else if (door1->type == 0 && door2->type == 0)
    {
        if ((door1->y > door2->y))
        {
            Door *tmp = (Door *)malloc(sizeof(Door));
            tmp->type = door1->type;
            tmp->x = door1->x;
            tmp->y = door1->y;
            door1 = door2;
            door2 = tmp;
        }
        int y = door1->y + (door1->type ? 1 : -1);
        mvprintw(y, door1->x, "#");
        int destX = door2->x;
        int x;
        for (x = door1->x; x != destX; x += (door1->x < door2->x) ? 1 : -1)
        {
            mvprintw(y, x, "#");
        }
        for (; y != door2->y; y += (door1->y < door2->y) ? 1 : -1)
        {
            mvprintw(y, x, "#");
        }
        mvprintw(door2->y, door2->x, "+");
    }
    else
    {
        int y = door1->y + (door1->type ? 1 : -1);
        mvprintw(y, door1->x, "#");
        int destX = door2->x;
        int x;
        for (x = door1->x; x != destX; x += (door1->x < door2->x) ? 1 : -1)
        {
            mvprintw(y, x, "#");
        }
        for (; y != door2->y; y += (door1->y < door2->y) ? 1 : -1)
        {
            mvprintw(y, x, "#");
        }
        mvprintw(door2->y, door2->x, "+");
    }
}

void printRoom(Room *room , int i)
{
    int x, y, width, height;
    x = room->startX;
    y = room->startY;
    height = room->height;
    width = room->width;
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
        if (i == 5 && j == 1)
            continue;
        mvprintw(room->doors[j]->y, room->doors[j]->x, "+");
    }
    refresh();
}

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
        y = rangeY + 17 - height - 5;
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
    room->doors[0]->type = 0;
    room->doors[1]->type = 1;

    room->startX = x;
    room->startY = y;
    room->height = height;
    room->width = width;
}

void movePlayer(Player *player, int x, int y)
{
    char c = mvinch(y, x);
    if (c != '-' && c != '|' && x >= 0 && x <= maxX && y > 3 && y <= maxY)
    {
        mvprintw(player->y, player->x, c == '.' ? "." : c == '+' ? "."
                                                                 : "#");
        player->x = x;
        player->y = y;
        mvprintw(player->y, player->x, "@");
        refresh();
    }
}

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    printf("%d , %d\n", maxX, maxY);
    noecho();
    Player player;
    Room **rooms = (Room **)malloc(6 * sizeof(Room *));
    int rangeX = 7, rangeY = 3;
    for (int i = 0; i < 6; i++)
    {
        rooms[i] = (Room *)malloc(sizeof(Room));
        createRoom(rooms[i], (i % 3) * 49 + rangeX, (i % 2) * 18 + rangeY);
        if (i == 0 || i == 5)
        {
            rooms[i]->doorCount = 1;
        }
        else
        {
            rooms[i]->doorCount = 2;
        }
        int x, y, width, height;
        x = rooms[i]->startX;
        y = rooms[i]->startY;
        height = rooms[i]->height;
        width = rooms[i]->width;
        rooms[i]->isVisible = i == 0 ? true : false;
    }
    for (int i = 0; i < 6; i++)
    {
        if(rooms[i]->isVisible){
            printRoom(rooms[i] , i);
        }
    }
    
    player.x = rooms[0]->startX + 1;
    player.y = rooms[0]->startY + 1;
    mvprintw(player.y, player.x, "@");
    refresh();
    // connectRooms(rooms[0]->doors[1], rooms[3]->doors[0]);
    // connectRooms(rooms[3]->doors[1], rooms[1]->doors[1]);
    // connectRooms(rooms[1]->doors[0], rooms[4]->doors[1]);
    // connectRooms(rooms[4]->doors[0], rooms[2]->doors[0]);
    // connectRooms(rooms[2]->doors[1], rooms[5]->doors[0]);
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
