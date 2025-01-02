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
    int index;
    int visiblePoint;
} Passway;

typedef struct
{
    Point cord;
    char type; // 'n' -> normal 's' -> secret 'l' -> locked
} Door;

typedef struct
{
    Point cord;
    int height;
    int width;
    Door *doors;
    int doorCount;
    Point window;
    bool isVisible;
    int index;
} Room;

typedef struct
{
    Point cord;
    int health;
    int state; // 0 -> in passway ; 1 -> in room
    Room *room;
    Passway *passway;
    int foodCount;
    int level ;
} Player;

typedef struct
{
    Room **rooms;
    Passway **passways;
    Player *player;
    int roomsCount;
    int level;
} Level;

typedef struct
{
    Level **levels;
    int currentLevel;
} Game;

void handleMove(Level *);
void movePlayer(Player *player, Room **rooms, Passway **passways, int roomsCount, int x, int y);
Room *createRoom(Room **rooms, int roomsCount, int min_x, int min_y, int max_X, int max_y);
bool hasOverlap(Room a, Room b);
void printDoors(Room *room);
void printRoom(Room *room);
int distance(Point a, Point b);
bool isInRoom(Room *room, Point p);
void createPassway(Passway **passway, Room **rooms, int roomCount);
void showLevel(Level *level);
void showPass(Passway *);
void showPlayeInfo(Player player);

extern int maxY,
    maxX;

int mapMode = 0;
WINDOW *mapWin;
Game *game;

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    // printf("%d , %d\n", maxX, maxY);
    noecho();

    // initilizing rooms
    int roomsCounts = randomNumber(6, 8);
    if (roomsCounts == 7)
    {
        roomsCounts = 6;
    }
    // int roomsCounts = 8;
    Room **rooms = (Room **)malloc(roomsCounts * sizeof(Room *));

    for (int i = 0; i < roomsCounts; i++)
    {
        if (i < roomsCounts / 2)
        {
            int min_x = 4 + (i % (roomsCounts / 2)) * (maxX - 10) / (roomsCounts / 2);
            int max_x = min_x + (maxX - 10) / (roomsCounts / 2);
            int min_y = 4;
            int max_y = maxY / 2 - 4;
            rooms[i] = createRoom(rooms, i, min_x, min_y, max_x, max_y);
            if (i == 0 || i == 5)
            {
                rooms[i]->doorCount = 1;
            }
        }
        else
        {
            int min_x = 4 + (i % (roomsCounts / 2)) * (maxX - 10) / (roomsCounts / 2);
            int max_x = min_x + (maxX - 10) / (roomsCounts / 2);
            int min_y = maxY / 2 + 4;
            int max_y = maxY - 4;
            rooms[i] = createRoom(rooms, i, min_x, min_y, max_x, max_y);
            if (i == 0 || i == roomsCounts - 1)
            {
                rooms[i]->doorCount = 1;
            }
        }
        if (i == 0)
        {
            rooms[0]->isVisible = true;
        }
        rooms[i]->index = i;
    }

    // initializing passways
    Passway **passways = (Passway **)malloc((roomsCounts - 1) * sizeof(Passway *));
    for (int i = 0; i < roomsCounts - 1; i++)
    {
        passways[i] = (Passway *)malloc(sizeof(Passway));
        passways[i]->index = i;
        passways[i]->visiblePoint = 0;
    }

    createPassway(passways, rooms, roomsCounts);
    refresh();

    // initialize player
    Player player;
    player.cord.x = rooms[0]->cord.x + 2;
    player.cord.y = rooms[0]->cord.y + 2;
    player.state = 1;
    player.room = rooms[0];
    player.passway = NULL;
    player.health = 30;
    player.foodCount = 0;
    player.level = 0;

    // creating first Level
    Level *firstLevel = (Level *)malloc(sizeof(Level));
    firstLevel->level = 1;
    firstLevel->rooms = rooms;
    firstLevel->passways = passways;
    firstLevel->player = &player;
    firstLevel->roomsCount = roomsCounts;

    // initilizing the game
    game = (Game *)malloc(sizeof(Game));
    game->levels = (Level **)malloc(4 * sizeof(Level *));
    game->levels[0] = firstLevel;
    game->currentLevel = 0;

    // adding foods , traps , stairs 

    showLevel(firstLevel);

    keypad(stdscr, true);

    // checking key pressing
    handleMove(firstLevel);

    // breaking the game
    Mix_FreeMusic(music);
    return;
};

void showPlayeInfo(Player player){
    mvprintw(1,maxX/2, "Level : %d   foodsCount : %d   health : %d" , player.level , player.foodCount , player.health);
    refresh();
}

void showLevel(Level *level)
{
    // show rooms
    for (int i = 0; i < level->roomsCount; i++)
    {
        if (level->rooms[i]->isVisible)
        {
            printRoom(level->rooms[i]);
        }
    }

    // show passways
    for (int i = 0; i < level->roomsCount - 1; i++)
    {
        showPass(level->passways[i]);
    }

    // show player
    mvprintw(level->player->cord.y, level->player->cord.x, "@");

    // show player info
    showPlayeInfo(*level->player);
    refresh();
}

bool hasOverlap(Room a, Room b)
{
    return !(a.cord.x + a.width < b.cord.x - 4 || b.cord.x + b.width < a.cord.x - 4 || a.cord.y + a.height < b.cord.y - 1 || b.cord.y + b.height < a.cord.y - 1);
}

void createPassway(Passway **passways, Room **rooms, int roomCount)
{
    if (roomCount == 6)
    {
        {
            passways[0]->from = 0;
            passways[0]->to = 1;
            passways[0]->count = 0;
            passways[0]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[0]->doors[0].cord;
            Point b = rooms[1]->doors[0].cord;
            Point current = a;
            while (current.y != (maxY / 2) - 2)
            {
                current.y += ((maxY / 2) - 2 > current.y) ? 1 : -1;
                passways[0]->points[passways[0]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[0]->points[passways[0]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[0]->points[passways[0]->count++] = current;
            }
        }
        {
            passways[1]->from = 0;
            passways[1]->to = 1;
            passways[1]->count = 0;
            passways[1]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[1]->doors[1].cord;
            Point b = rooms[2]->doors[1].cord;
            Point current = a;
            while (current.y != 4)
            {
                current.y += (4 > current.y) ? 1 : -1;
                passways[1]->points[passways[1]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[1]->points[passways[1]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[1]->points[passways[1]->count++] = current;
            }
        }
        {
            passways[2]->from = 0;
            passways[2]->to = 1;
            passways[2]->count = 0;
            passways[2]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[2]->doors[0].cord;
            Point b = rooms[3]->doors[1].cord;
            Point current = a;
            while (current.y != maxY / 2)
            {
                current.y += (maxY / 2 > current.y) ? 1 : -1;
                passways[2]->points[passways[2]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[2]->points[passways[2]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[2]->points[passways[2]->count++] = current;
            }
        }
        {
            passways[3]->from = 0;
            passways[3]->to = 1;
            passways[3]->count = 0;
            passways[3]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[3]->doors[0].cord;
            Point b = rooms[4]->doors[0].cord;
            Point current = a;
            while (current.y != maxY - 2)
            {
                current.y += (maxY - 2 > current.y) ? 1 : -1;
                passways[3]->points[passways[3]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[3]->points[passways[3]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[3]->points[passways[3]->count++] = current;
            }
        }
        {
            passways[4]->from = 0;
            passways[4]->to = 1;
            passways[4]->count = 0;
            passways[4]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[4]->doors[1].cord;
            Point b = rooms[5]->doors[0].cord;
            Point current = a;
            while (current.y != maxY / 2 + 2)
            {
                current.y += (maxY / 2 + 2 > current.y) ? 1 : -1;
                passways[4]->points[passways[4]->count++] = current;
            }
            while (current.x != maxX - 4)
            {
                current.x += (maxX - 4 > current.x) ? 1 : -1;
                passways[4]->points[passways[4]->count++] = current;
            }
            while (current.y != b.y + 1)
            {
                current.y += (b.y + 1 > current.y) ? 1 : -1;
                passways[4]->points[passways[4]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[4]->points[passways[4]->count++] = current;
            }
            current.y -= 1;
            passways[4]->points[passways[4]->count++] = current;
        }
    }
    else
    {
        {
            passways[0]->from = 0;
            passways[0]->to = 1;
            passways[0]->count = 0;
            passways[0]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[0]->doors[0].cord;
            Point b = rooms[1]->doors[0].cord;
            Point current = a;
            while (current.y != (maxY / 2) - 2)
            {
                current.y += ((maxY / 2) - 2 > current.y) ? 1 : -1;
                passways[0]->points[passways[0]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[0]->points[passways[0]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[0]->points[passways[0]->count++] = current;
            }
        }
        {
            passways[1]->from = 0;
            passways[1]->to = 1;
            passways[1]->count = 0;
            passways[1]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[1]->doors[1].cord;
            Point b = rooms[2]->doors[1].cord;
            Point current = a;
            while (current.y != 4)
            {
                current.y += (4 > current.y) ? 1 : -1;
                passways[1]->points[passways[1]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[1]->points[passways[1]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[1]->points[passways[1]->count++] = current;
            }
        }
        {
            passways[2]->from = 0;
            passways[2]->to = 1;
            passways[2]->count = 0;
            passways[2]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[2]->doors[0].cord;
            Point b = rooms[3]->doors[0].cord;
            Point current = a;
            while (current.y != maxY / 2 - 3)
            {
                current.y += (maxY / 2 - 3 > current.y) ? 1 : -1;
                passways[2]->points[passways[2]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[2]->points[passways[2]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[2]->points[passways[2]->count++] = current;
            }
        }
        {
            passways[3]->from = 0;
            passways[3]->to = 1;
            passways[3]->count = 0;
            passways[3]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[3]->doors[1].cord;
            Point b = rooms[4]->doors[1].cord;
            Point current = a;
            current.y -= 1;
            passways[3]->points[passways[3]->count++] = current;
            while (current.x != maxX - 5)
            {
                current.x += (maxX - 5 > current.x) ? 1 : -1;
                passways[3]->points[passways[3]->count++] = current;
            }
            while (current.y != maxY / 2)
            {
                current.y += (maxY / 2 > current.y) ? 1 : -1;
                passways[3]->points[passways[3]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[3]->points[passways[3]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[3]->points[passways[3]->count++] = current;
            }
        }
        {
            passways[4]->from = 0;
            passways[4]->to = 1;
            passways[4]->count = 0;
            passways[4]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[4]->doors[0].cord;
            Point b = rooms[5]->doors[0].cord;
            Point current = a;
            while (current.y != maxY - 2)
            {
                current.y += (maxY - 2 > current.y) ? 1 : -1;
                passways[4]->points[passways[4]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[4]->points[passways[4]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[4]->points[passways[4]->count++] = current;
            }
        }
        {
            passways[5]->from = 0;
            passways[5]->to = 1;
            passways[5]->count = 0;
            passways[5]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[5]->doors[1].cord;
            Point b = rooms[6]->doors[1].cord;
            Point current = a;
            while (current.y != maxY / 2 + 2)
            {
                current.y += (maxY / 2 + 2 > current.y) ? 1 : -1;
                passways[5]->points[passways[5]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[5]->points[passways[5]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[5]->points[passways[5]->count++] = current;
            }
        }
        {
            passways[6]->from = 0;
            passways[6]->to = 1;
            passways[6]->count = 0;
            passways[6]->points = (Point *)malloc(1000 * sizeof(Point));
            Point a = rooms[6]->doors[0].cord;
            Point b = rooms[7]->doors[0].cord;
            Point current = a;
            while (current.y != maxY - 2)
            {
                current.y += (maxY - 2 > current.y) ? 1 : -1;
                passways[6]->points[passways[6]->count++] = current;
            }
            while (current.x != b.x)
            {
                current.x += (b.x > current.x) ? 1 : -1;
                passways[6]->points[passways[6]->count++] = current;
            }
            while (current.y != b.y)
            {
                current.y += (b.y > current.y) ? 1 : -1;
                passways[6]->points[passways[6]->count++] = current;
            }
        }
    }
}

int distance(Point a, Point b)
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

void printDoors(Room *room)
{
    WINDOW *screen = mapMode ? mapWin : stdscr;

    for (int i = 0; i < room->doorCount; i++)
    {
        mvwprintw(screen, room->doors[i].cord.y, room->doors[i].cord.x, "+");
    }
}

Room *createRoom(Room **rooms, int roomsCount, int min_x, int min_y, int max_x, int max_y)
{
    Room *room = (Room *)malloc(sizeof(Room));

    bool validPlacement;
    do
    {
        validPlacement = true;
        room->width = randomNumber(MIN_WIDTH, MAX_WIDTH);
        room->height = randomNumber(MIN_HEIGHT, MAX_HEIGHT);
        room->cord.x = randomNumber(min_x, max_x - room->width);
        room->cord.y = randomNumber(min_y, max_y - room->height);
        for (int i = 0; i < roomsCount; i++)
        {
            if (hasOverlap(*room, *rooms[i]))
            {
                validPlacement = false;
                break;
            }
        }
    } while (!validPlacement);
    room->isVisible = false;
    room->doorCount = 2;
    room->doors = (Door *)malloc(sizeof(Door) * room->doorCount);
    for (int i = 0; i < room->doorCount; i++)
    {
        if (i % 2 == 0)
        {
            room->doors[i].cord.x = room->cord.x + 1 + randomNumber(0, room->width - 2);
            room->doors[i].cord.y = room->cord.y + room->height - 1;
        }
        else
        {
            room->doors[i].cord.x = room->cord.x + 1 + randomNumber(0, room->width - 2);
            room->doors[i].cord.y = room->cord.y + 1;
        }
    }
    return room;
}

bool isInRoom(Room *room, Point p)
{
    return p.x >= room->cord.x && p.x <= (room->cord.x + room->width) && p.y >= room->cord.y && p.y <= (room->cord.y + room->height);
}

void handleMove(Level *level)
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
            movePlayer(level->player, level->rooms, level->passways, level->roomsCount, level->player->cord.x, level->player->cord.y - 1);
        }
        else if (c == 's' || c == '2')
        {
            movePlayer(level->player, level->rooms, level->passways, level->roomsCount, level->player->cord.x, level->player->cord.y + 1);
        }
        else if (c == 'd' || c == '6')
        {
            movePlayer(level->player, level->rooms, level->passways, level->roomsCount, level->player->cord.x + 1, level->player->cord.y);
        }
        else if (c == 'a' || c == '4')
        {
            movePlayer(level->player, level->rooms, level->passways, level->roomsCount, level->player->cord.x - 1, level->player->cord.y);
        }
        else if (c == 'm')
        {

            mapWin = newwin(maxY, maxX, 0, 0);
            mvwprintw(mapWin, 1, 1, "Map Mode !!");
            wrefresh(mapWin);
            mapMode = 1;
            for (int i = 0; i < level->roomsCount; i++)
            {
                printRoom(level->rooms[i]);
            }
            wrefresh(mapWin);
            getchar();
            wclear(mapWin);
            wrefresh(mapWin);
            mapMode = 0;
            showLevel(level);
            refresh();
        }
    }
}

void showPass(Passway *passway)
{
    for (int i = 0; i < (passway->visiblePoint > passway->count ? passway->count : passway->visiblePoint); i++)
    {
        mvprintw(passway->points[i].y, passway->points[i].x, "#");
    }
}

int inRoom(Room **rooms, int roomsCount, Point p)
{
    for (int i = 0; i < roomsCount; i++)
    {
        if (isInRoom(rooms[i], p))
        {
            return i;
        }
    }
    return -1;
}

void movePlayer(Player *player, Room **rooms, Passway **passways, int roomsCount, int x, int y)
{
    char c = mvinch(y, x);
    if (c == '+')
    {
        if (player->passway == NULL)
        {
            player->passway = passways[0];
            player->passway->visiblePoint = 4;
            showPass(player->passway);
            player->passway->visiblePoint += 1;
        }
        else
        {
            if (player->passway->index != roomsCount - 2)
            {
                player->passway = passways[player->passway->index + 1];
                player->passway->visiblePoint = 4;
                showPass(player->passway);
                player->passway->visiblePoint += 1;
            }
        }
        refresh();
    }
    if (c == '#')
    {
        showPass(player->passway);
        player->passway->visiblePoint += 1;
        if (player->passway->visiblePoint == player->passway->count)
        {
            if (player->room->index != roomsCount - 1)
            {
                printRoom(rooms[player->room->index + 1]);
            }
        }
    }
    if ((c == '.' || c == '+' || c == '#') && x >= 0 && x <= maxX && y > 3 && y <= maxY)
    {
        mvprintw(player->cord.y, player->cord.x, c == '.' ? "." : c == '+' ? "."
                                                                           : "#");
        player->cord.x = x;
        player->cord.y = y;
        mvprintw(player->cord.y, player->cord.x, "@");
        refresh();
    }
    int index = inRoom(rooms, roomsCount, player->cord);
    if (index != -1)
    {
        player->room = rooms[index];
        rooms[index]->isVisible = true;
        player->state = 1;
    }
}

void printRoom(Room *room)
{
    int x, y, width, height;
    x = room->cord.x;
    y = room->cord.y;
    height = room->height;
    width = room->width;
    WINDOW *screen = mapMode ? mapWin : stdscr;
    for (int j = x + 1; j < x + width; j++)
    {
        mvwprintw(screen, y + 1, j, "-");
        mvwprintw(screen, y + height - 1, j, "-");
    }
    for (int j = y + 2; j < y + height - 1; j++)
    {
        mvwprintw(screen, j, x, "|");
        mvwprintw(screen, j, x + width, "|");

        for (int k = x + 1; k < x + width; k++)
        {
            mvwprintw(screen, j, k, ".");
        }
    }
    printDoors(room);
    refresh();
}