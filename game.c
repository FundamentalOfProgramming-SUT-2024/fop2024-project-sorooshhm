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
    int from;      // index of source room
    int to;        // index of dest room
    Point *points; // (x,y) of the way
    int count;
} Passway;

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
    int state; // 0 -> in passway ; 1 -> in room
} Player;

typedef struct Node
{
    Point point;
    int g;
    int h;
    int f;
    struct Node *parent;
} Node;

Node *createNode(Point p, int g, int h, Node *parent)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->point = p;
    node->g = g;
    node->h = h;
    node->f = g + h;
    node->parent = parent;
    return node;
}
bool isValid(Point p, Room **rooms, int roomCount)
{
    if (p.x < 0 || p.x >= maxX || p.y < 4 || p.y >= maxY)
    {
        return false;
    }
    for (int i = 0; i < roomCount; i++)
    {
        if (p.x >= rooms[i]->cord.x && p.x < rooms[i]->cord.x + rooms[i]->width && p.y >= rooms[i]->cord.y && p.y < rooms[i]->cord.y + rooms[i]->height)
        {
            return false;
        }
    }
    return true;
}

int heuristic(Point a, Point b) { return abs(a.x - b.x) + abs(a.y - b.y); }
void handleMove(Player player);
void movePlayer(Player *player, int x, int y);
Room *createRoom(Room **rooms, int roomsCount);
bool hasOverlap(Room a, Room b);
void printDoors(Room *room);
void printRoom(Room *room, int);
void connectRooms(Point a, Point b, Passway *passway, Room **rooms, int roomsCount);
bool validatePoint(Point p, Room **rooms, int roomsCount);
bool hasPoint(Point *points, Point point, int count);

extern int maxY, maxX;

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    printf("%d , %d\n", maxX, maxY);
    noecho();
    curs_set(false);
    // initilizing rooms
    int roomsCounts = randomNumber(6, 8);
    Room **rooms = (Room **)malloc(roomsCounts * sizeof(Room *));
    for (int i = 0; i < roomsCounts; i++)
    {
        rooms[i] = createRoom(rooms, i);
        if (i == 0 || i == roomsCounts - 1)
        {
            rooms[i]->doorCount = 1;
        }
    }
    for (int i = 0; i < roomsCounts; i++)
    {
        printRoom(rooms[i], i);
    }

    // initializing passways
    Passway **passways = (Passway **)malloc((roomsCounts - 1) * sizeof(Passway *));
    // getchar();
    for (int i = 0; i < roomsCounts - 1; i++)
    {
        passways[i] = (Passway *)malloc(sizeof(Passway));
        passways[i]->count = 0;
        passways[i]->from = i;
        passways[i]->to = i + 1;
        passways[i]->points = (Point *)malloc(10000 * sizeof(Point));
        Point a = rooms[i]->doors[1];
        Point b = rooms[i + 1]->doors[0];
        connectRooms(a, b, passways[i], rooms, roomsCounts);
        mvprintw(1, 1, "finished");
        refresh();
    }

    Player player;
    player.state = 1;
    player.cord.x = rooms[0]->cord.x + 1;
    player.cord.y = rooms[0]->cord.y + 2;
    mvprintw(player.cord.y, player.cord.x, "@");
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
    return !(a.cord.x + a.width < b.cord.x - 4 || b.cord.x + b.width < a.cord.x - 4 || a.cord.y + a.height < b.cord.y - 1 || b.cord.y + b.height < a.cord.y - 1);
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
            room->doors[i].y = room->cord.y + 2 + randomNumber(0, room->height - 4);
        }
        else
        {
            room->doors[i].x = room->cord.x + 1 + randomNumber(0, room->width - 2);
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
    // char prev = mvinch(player->cord.y, player->cord.x);
    if (c != '-' && c != '|' && x >= 0 && x <= maxX && y > 3 && y <= maxY)
    {
        mvprintw(player->cord.y, player->cord.x, c == '.' ? "." : "#");
        player->cord.x = x;
        player->cord.y = y;
        mvprintw(player->cord.y, player->cord.x, "@");
        refresh();
    }
}

void connectRooms(Point a, Point b, Passway *passway, Room **rooms, int roomsCount)
{
    if (a.x == b.x && a.y == b.y)
        return;

    Point current = a;
    Point tmp = a;
    getchar();
    refresh();
    while (current.x != b.x)
    {
        current.x += (b.x > current.x) ? 1 : -1;
        if (!hasPoint(passway->points, current, passway->count) && validatePoint(current, rooms, roomsCount))
        {
            mvprintw(current.y, current.x, "#");
            mvprintw(1, 1, "1 (%d , %d )", current.x, current.y);
            passway->points[passway->count++] = current;
        }
    }
    while (current.y != b.y)
    {
        current.y += (b.y > current.y) ? 1 : -1;
        if (!hasPoint(passway->points, current, passway->count) && validatePoint(current, rooms, roomsCount))
        {
            mvprintw(current.y, current.x, "#");
            mvprintw(1, 1, "1 (%d , %d )", current.x, current.y);
            passway->points[passway->count++] = current;
        }
    }
    // current.x += (b.x > current.x) ? 1 : -1;
    // if (!hasPoint(passway->points, current, passway->count) && validatePoint(current, rooms, roomsCount))
    // {
    //     mvprintw(current.y, current.x, "#");
    //     mvprintw(1,1, "1 (%d , %d )", current.x, current.y);
    //     passway->points[passway->count++] = current;
    //     connectRooms(current, b, passway, rooms, roomsCount);
    // }
    // current = tmp;

    // current.y += (b.y > current.y) ? 1 : -1;
    // ;
    // if (!hasPoint(passway->points, current, passway->count) && validatePoint(current, rooms, roomsCount))
    // {
    //     mvprintw(current.y, current.x, "#");
    //     mvprintw(1,1, "2 (%d , %d )", current.x, current.y);
    //     passway->points[passway->count++] = current;
    //     connectRooms(current, b, passway, rooms, roomsCount);
    // }
    // current = tmp;
    // current.x -= 1;
    // mvprintw(2, 2, "3 (%d , %d )", current.x, current.y);
    // if (!hasPoint(passway->points, current, passway->count) && validatePoint(current, rooms, roomsCount))
    // {
    //     mvprintw(current.y, current.x, "3");
    //     passway->points[passway->count++] = current;
    //     connectRooms(current, b, passway, rooms, roomsCount);
    // }

    // current = tmp;
    // current.y -= 1;
    // mvprintw(2, 2, "4 (%d , %d )", current.x, current.y);

    // if (!hasPoint(passway->points, current, passway->count) && validatePoint(current, rooms, roomsCount))
    // {
    //     mvprintw(current.y, current.x, "4");
    //     passway->points[passway->count++] = current;
    //     connectRooms(current, b, passway, rooms, roomsCount);
    // }
    // while (current.x != b.x)
    // {
    //     current.x += (b.x > current.x) ? 1 : -1;
    //     if (validatePoint(current, rooms, roomsCount))
    //     {
    //         mvprintw(current.y, current.x, "#");
    //     }
    // }
    // while (current.y != b.y)
    // {
    //     current.y += (b.y > current.y) ? 1 : -1;
    //     if (validatePoint(current, rooms, roomsCount))
    //     {
    //         mvprintw(current.y, current.x, "#");
    //     }
    // }
    return;
}

bool validatePoint(Point p, Room **rooms, int roomsCount)
{
    if (p.x < 0 || p.x >= 135 || p.y < 4 || p.y >= 36)
        return false;
    for (int i = 0; i < roomsCount; i++)
    {
        Room room = *(rooms[i]);
        if (p.x > room.cord.x && p.x < (room.cord.x + room.width) && p.y > room.cord.y && p.y < (room.cord.y + room.height))
        {
            return false;
        }
    }
    return true;
}

bool hasPoint(Point *points, Point point, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (point.x == points[i].x && point.y == points[i].y)
            return true;
    }
    return false;
}

void printRoom(Room *room, int i)
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
            mvprintw(j, k, "%d", i + 1);
        }
    }
    printDoors(room);
    refresh();
}