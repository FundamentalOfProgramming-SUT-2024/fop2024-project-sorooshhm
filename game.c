#include <ncurses.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include "auth.h"
#include "utils.h"

#define MAX_HEIGHT 8
#define MIN_HEIGHT 5
#define MAX_WIDTH 14
#define MIN_WIDTH 6

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
    return !(a.cord.x + a.width <= b.cord.x || b.cord.x + b.width <= a.cord.x || a.cord.y + a.height <= b.cord.y || b.cord.y + b.height <= a.cord.y);
}

Room *createRoom(Room **rooms, int roomsCount)
{
    Room *room = (Room *)malloc(sizoef(Room));

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
            if (isOverlap(*room, *rooms[i]))
            {
                validPlacement = false;
                break;
            }
        }
    } while (!validPlacement);
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
