#include <ncurses.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include "auth.h"
#include "utils.h"

#define MAX_HEIGHT 10
#define MIN_HEIGHT 6
#define MAX_WIDTH 14
#define MIN_WIDTH 8

typedef struct
{
    int x;
    int y;
} Point;

typedef struct
{
    Point cord;
    int height;
    int width;
    Point *doors;
    int doorCount;
    Point window;
    bool isVisible;
} Room;

typedef struct
{
    Point cord;
    int health;
} Player;

void handleMove(Player player);
void movePlayer(Player *player, int x, int y);
Room *createRoom(Room **rooms, int roomsCount);
bool hasOverlap(Room a, Room b);
void printDoors(Room *room);
void printRoom(Room *room);

extern int maxY, maxX;

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    printf("%d , %d\n", maxX, maxY);
    noecho();
    // initilizing rooms
    int roomsCounts = randomNumber(6, 8);
    Room **rooms = (Room **)malloc(roomsCounts * sizeof(Room *));
    for (int i = 0; i < roomsCounts; i++)
    {
        rooms[i] = createRoom(rooms, i);
    }
    for (int i = 0; i < roomsCounts; i++)
    {
        printRoom(rooms[i]);
    }

    Player player;
    refresh();
    keypad(stdscr, true);

    // checking key pressing
    handleMove(player);

    // breaking the game
    Mix_FreeMusic(music);
    return;
};

bool hasOverlap(Room a, Room b)
{
    return !(a.cord.x + a.width < b.cord.x  || b.cord.x + b.width < a.cord.x || a.cord.y + a.height < b.cord.y || b.cord.y + b.height < a.cord.y);
}

void printDoors(Room *room)
{
    for (int i = 0; i < room->doorCount; i++)
    {
        mvprintw(room->doors[i].y, room->doors[i].x, "+");
    }
}

Room *createRoom(Room **rooms, int roomsCount)
{
    Room *room = (Room *)malloc(sizeof(Room));

    bool validPlacement;
    do
    {
        validPlacement = true;
        room->width = randomNumber(MIN_WIDTH, MAX_WIDTH);
        room->height = randomNumber(MIN_HEIGHT, MAX_HEIGHT);
        room->cord.x = randomNumber(4, maxX - room->width - 10);
        room->cord.y = randomNumber(4, maxY - room->height - 5);
        for (int i = 0; i < roomsCount; i++)
        {
            if (hasOverlap(*room, *rooms[i]))
            {
                validPlacement = false;
                break;
            }
        }
    } while (!validPlacement);
    room->isVisible = true;
    room->doorCount = 2;
    room->doors = (Point *)malloc(sizeof(Point) * room->doorCount);
    for (int i = 0; i < room->doorCount; i++)
    {
        if (i % 2 == 0)
        {
            room->doors[i].x = room->cord.x;
            room->doors[i].y = room->cord.y +2 + randomNumber(0, room->height - 3);
        }
        else
        {
            room->doors[i].x = room->cord.x + randomNumber(0, room->width - 1);
            room->doors[i].y = room->cord.y + 1;
        }
    }
    return room;
}

void handleMove(Player player)
{
    while (1)
    {
        char c = getchar();
        if (c == 27)
        {
            break;
        }
        else if (c == 'w' || c == '8')
        {
            movePlayer(&player, player.cord.x, player.cord.y - 1);
        }
        else if (c == 's' || c == '2')
        {
            movePlayer(&player, player.cord.x, player.cord.y + 1);
        }
        else if (c == 'd' || c == '6')
        {
            movePlayer(&player, player.cord.x + 1, player.cord.y);
        }
        else if (c == 'a' || c == '4')
        {
            movePlayer(&player, player.cord.x - 1, player.cord.y);
        }
    }
}

void movePlayer(Player *player, int x, int y)
{
    char c = mvinch(y, x);
    if (c != '-' && c != '|' && x >= 0 && x <= maxX && y > 3 && y <= maxY)
    {
        mvprintw(player->cord.y, player->cord.x, c == '.' ? "." : c == '+' ? "."
                                                                           : "#");
        player->cord.x = x;
        player->cord.y = y;
        mvprintw(player->cord.y, player->cord.x, "@");
        refresh();
    }
}

void printRoom(Room *room)
{
    int x, y, width, height;
    x = room->cord.x;
    y = room->cord.y;
    height = room->height;
    width = room->width;
    for (int j = x + 1; j < x + width; j++)
    {
        mvprintw(y + 1, j, "-");
        mvprintw(y + height - 1, j, "-");
    }
    for (int j = y + 2; j < y + height - 1; j++)
    {
        mvprintw(j, x, "|");
        mvprintw(j, x + width, "|");

        for (int k = x + 1; k < x + width; k++)
        {
            mvprintw(j, k, ".");
        }
    }
    printDoors(room);
    refresh();
}