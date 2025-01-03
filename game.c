#include <ncurses.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "auth.h"
#include "utils.h"
#include "game.h"
#include "menu.h"
#include "form.h"
#include "db.h"

#define MAX_HEIGHT 10
#define MIN_HEIGHT 8
#define MAX_WIDTH 14
#define MIN_WIDTH 9

void handleMove();
void movePlayer(Player *player, Room **rooms, Passway **passways, int roomsCount, int x, int y);
Room *createRoom(Room **rooms, int roomsCount, int min_x, int min_y, int max_X, int max_y);
bool hasOverlap(Room a, Room b);
void printDoors(Room *room);
void printRoom(Room *room);
int distance(Point a, Point b);
bool isInRoom(Room *room, Point p);
void createPassway(Passway **passway, Room **rooms, int roomCount);
int inRoom(Room **rooms, int roomsCount, Point p);
int inPassway(Passway **passways, int, Point p);
void showLevel(Level *level);
void showPass(Passway *);
void showPlayeInfo(Player player);
void *damagePlayer(void *player);
void lose();
void createLevel(Level *level, int);
int changeLevel(Stair stair);

extern int maxY,
    maxX;

int mapMode = 0;
int damageTime = 30;
WINDOW *mapWin;
Game *game;
Player *player;

void startGame(User *user, Mix_Music *music)
{
    srand(time(NULL));
    // printf("%d , %d\n", maxX, maxY);
    noecho();

    // initilizing the game
    game = (Game *)malloc(sizeof(Game));
    game->levels = (Level **)malloc(4 * sizeof(Level *));
    for (int i = 0; i < 4; i++)
    {
        game->levels[i] = (Level *)malloc(sizeof(Level));
        createLevel(game->levels[i], i);
    }
    game->currentLevel = 0;

    // initialize player
    player = (Player *)malloc(sizeof(Player));
    player->cord.x = game->levels[game->currentLevel]->rooms[0]->cord.x + 2;
    player->cord.y = game->levels[game->currentLevel]->rooms[0]->cord.y + 2;
    player->state = 1;
    player->room = game->levels[game->currentLevel]->rooms[0];
    player->passway = NULL;
    player->health = 30;
    player->foodCount = 0;
    player->level = 0;
    player->name = user->username;
    player->usedFood = (int *)calloc(50, sizeof(int));
    player->foods = (Food **)malloc(40 * sizeof(Food));
    game->player = player;

    showLevel(game->levels[game->currentLevel]);

    keypad(stdscr, true);

    // damage the player
    pthread_t damageThread;
    pthread_create(&damageThread, NULL, damagePlayer, NULL);
    // pthread_join(damageThread, NULL);

    // checking key pressing
    handleMove(game->levels[game->currentLevel]);

    // pthread_exit(NULL);
    // saving the game
    saveGame(game, user);
    clear();
    refresh();
    // breaking the game
    Mix_FreeMusic(music);
    return;
};

int inPassway(Passway **passways, int count, Point p)
{
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < passways[i]->count; j++)
        {
            if (passways[i]->points[j].x == p.x && passways[i]->points[j].y == p.y)
            {
                return i;
            }
        }
    }
    return -1;
}

void resumeGame(User *user, Mix_Music *music)
{
    game = (Game *)malloc(sizeof(Game));
    loadGame(game, user);
    int roomIndex = inRoom(game->levels[game->currentLevel]->rooms, game->levels[game->currentLevel]->roomsCount, game->player->cord);
    if (roomIndex != -1)
    {
        game->player->room = game->levels[game->currentLevel]->rooms[roomIndex];
        if (roomIndex != 0)
        {
            game->player->passway = game->levels[game->currentLevel]->passways[roomIndex - 1];
        }
        else
        {
            game->player->passway = NULL;
        }
    }
    else
    {
        int passwayIndex = inPassway(game->levels[game->currentLevel]->passways, game->levels[game->currentLevel]->roomsCount - 1, game->player->cord);
        if (passwayIndex == -1)
        {
            game->player->passway = NULL;
        }
        else
        {
            game->player->room = game->levels[game->currentLevel]->rooms[passwayIndex];
            game->player->passway = game->levels[game->currentLevel]->passways[passwayIndex];
        }
    }
    game->player->level = game->currentLevel;
    // mvprintw(1, 1, "passway index %d", game->player->passway->index);
    // refresh();

    player = game->player;

    for (int i = game->currentLevel + 1; i < 4; i++)
    {
        game->levels[i] = (Level *)malloc(sizeof(Level));
        createLevel(game->levels[i], i);
    }

    showLevel(game->levels[game->currentLevel]);

    keypad(stdscr, true);

    // damage the player
    pthread_t damageThread;
    pthread_create(&damageThread, NULL, damagePlayer, NULL);
    // pthread_join(damageThread, NULL);

    // checking key pressing
    handleMove(game->levels[game->currentLevel]);

    // pthread_exit(NULL);
    // saving the game
    saveGame(game, user);
    clear();
    refresh();
    // breaking the game
    Mix_FreeMusic(music);
    // pthread_join(damageThread, NULL);
    return;
}

int changeLevel(Stair stair)
{
    int sourceIndex = stair.from;
    int destIndex = stair.to;
    if (destIndex > sourceIndex)
    {
        if (game->levels[destIndex] == NULL)
        {
            return 0;
        }
        game->currentLevel = destIndex;
        player->cord.x = game->levels[game->currentLevel]->rooms[0]->cord.x + 2;
        player->cord.y = game->levels[game->currentLevel]->rooms[0]->cord.y + 2;
        player->room = game->levels[game->currentLevel]->rooms[0];
        player->passway = NULL;
        player->level = game->currentLevel;
        clear();
        refresh();
        showLevel(game->levels[game->currentLevel]);
    }
    else
    {
        if (game->levels[destIndex] == NULL)
        {
            return 0;
        }
        game->currentLevel = destIndex;
        player->cord.x = game->levels[game->currentLevel]->rooms[game->levels[game->currentLevel]->roomsCount - 1]->cord.x + 2;
        player->cord.y = game->levels[game->currentLevel]->rooms[game->levels[game->currentLevel]->roomsCount - 1]->cord.y + 2;
        player->room = game->levels[game->currentLevel]->rooms[0];
        player->passway = game->levels[game->currentLevel]->passways[game->levels[game->currentLevel]->roomsCount - 2];
        player->level = game->currentLevel;
        clear();
        refresh();
        showLevel(game->levels[game->currentLevel]);
    }
    return 1;
}

void createLevel(Level *level, int levelIndex)
{
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
        if(i==roomsCounts-1){
            rooms[i]->doors[0].type = 'n';
            rooms[i]->doors[1].type = 'n';
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

    // adding foods , traps , stairs
    for (int i = 0; i < roomsCounts; i++)
    {
        rooms[i]->foodCount = 0;
        rooms[i]->trapCount = 0;
        rooms[i]->stairCount = 0;
        int num = rand();
        if (num % 3 == 0)
        {
            int count = randomNumber(2, 4);
            rooms[i]->foodCount = count;
            rooms[i]->foods = (Food *)malloc(count * sizeof(Food));
            for (int j = 0; j < count; j++)
            {
                rooms[i]->foods[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                rooms[i]->foods[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                rooms[i]->foods[j].health = 3;
                rooms[i]->foods[j].isUsed = false;
            }
        }
        if (num % 3 == 1)
        {
            int count = randomNumber(1, 3);
            rooms[i]->trapCount = count;
            rooms[i]->traps = (Trap *)malloc(count * sizeof(Trap));
            for (int j = 0; j < count; j++)
            {
                rooms[i]->traps[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                rooms[i]->traps[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                rooms[i]->traps[j].isVisible = false;
            }
        }
        if (i == 0 && levelIndex != 0)
        {
            rooms[i]->stairCount = 1;
            rooms[i]->stair.cord.x = rooms[i]->cord.x + rooms[i]->width - 2;
            rooms[i]->stair.cord.y = rooms[i]->cord.y + 2;
            rooms[i]->stair.from = levelIndex;
            rooms[i]->stair.to = levelIndex - 1;
        }
        if (i == roomsCounts - 1 && levelIndex != 3)
        {
            rooms[i]->stairCount = 1;
            rooms[i]->stair.cord.x = rooms[i]->cord.x + rooms[i]->width - 2;
            rooms[i]->stair.cord.y = rooms[i]->cord.y + 2;
            rooms[i]->stair.from = levelIndex;
            rooms[i]->stair.to = levelIndex + 1;
        }
    }
    level->level = levelIndex;
    level->rooms = rooms;
    level->passways = passways;
    level->roomsCount = roomsCounts;
}

void lose()
{
    WINDOW *lostWin = newwin(maxY, maxX, 0, 0);
    mvwprintw(lostWin, maxY / 2, 50, "RIP dear %s :(( ...", player->name);
    wrefresh(lostWin);
    sleep(5);
    wclear(lostWin);
    clear();
}

void *damagePlayer(void *args)
{
    while (1)
    {
        sleep(damageTime);
        (player)->health -= 5;
        showPlayeInfo(*player);
        if (player->health <= 0)
        {
            break;
        }
    }
    pthread_exit(NULL);
}

void showPlayeInfo(Player player)
{
    mvprintw(1, maxX / 2, "                                                             ");
    refresh();
    mvprintw(1, maxX / 2, "Level : %d   foodsCount : %d   health : %d", player.level + 1, player.foodCount, player.health);
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
    mvprintw(game->player->cord.y, game->player->cord.x, "@");

    // show player info
    showPlayeInfo(*game->player);
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
        if (room->doors[i].type != 'l')
        {
            mvwprintw(screen, room->doors[i].cord.y, room->doors[i].cord.x, "+");
        }
        else
        {
            attron(COLOR_PAIR(1));
            mvwprintw(screen, room->doors[i].cord.y, room->doors[i].cord.x, "@");
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(2));
            mvwprintw(screen, room->cord.y + 3, room->cord.x + 1, "&");
            attroff(COLOR_PAIR(2));
        }
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
    int num = rand();
    for (int i = 0; i < room->doorCount; i++)
    {
        if (num % 5 == 3)
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
            room->doors[i].type = 'l';
            room->doors[i].password = 0;
        }
        else
        {
            if (i % 2 == 0)
            {
                room->doors[i].cord.x = room->cord.x + 1 + randomNumber(0, room->width - 2);
                room->doors[i].cord.y = room->cord.y + room->height - 1;
                room->doors[i].type = 'n';
            }
            else
            {
                room->doors[i].cord.x = room->cord.x + 1 + randomNumber(0, room->width - 2);
                room->doors[i].cord.y = room->cord.y + 1;
                room->doors[i].type = 'n';
            }
        }
    }
    return room;
}

bool isInRoom(Room *room, Point p)
{
    return p.x > room->cord.x && p.x < (room->cord.x + room->width) && p.y > room->cord.y && p.y < (room->cord.y + room->height);
}

void handleMove()
{
    while (1)
    {
        Level *level = game->levels[game->currentLevel];
        if (player->health <= 0)
        {
            lose();
            break;
        }
        char c = getchar();
        if (c == 27)
        {
            break;
        }
        else if (c == 'w' || c == '8')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x, game->player->cord.y - 1);
        }
        else if (c == 's' || c == '2')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x, game->player->cord.y + 1);
        }
        else if (c == 'd' || c == '6')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x + 1, game->player->cord.y);
        }
        else if (c == 'a' || c == '4')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x - 1, game->player->cord.y);
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
        else if (c == 'e')
        {
            if (player->foodCount == 0)
            {
                continue;
            }
            WINDOW *menuWin = creaetMenuWindow(15, maxX / 2, maxY / 2 - 15, maxX / 4);

            char **menu = (char **)malloc(player->foodCount * sizeof(char *));
            for (int i = 0; i < player->foodCount; i++)
            {
                // if (!player.usedFood[i])
                // {
                menu[i] = malloc(20 * sizeof(char));
                sprintf(menu[i], "food %d", i + 1);
                // }
            }

            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Foods menu");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);
            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, player->foodCount, 0);
            player->usedFood[highlight] = 1;
            player->foodCount--;
            player->health += 3;
            getchar();
            wclear(menuWin);
            free(menu);
            clear();
            refresh();
            showLevel(level);
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

Food findFood(Point p, Room *room)
{
    for (int i = 0; i < room->foodCount; i++)
    {
        if (room->foods[i].cord.x == p.x && room->foods[i].cord.y == p.y)
        {
            room->foods[i].isUsed = true;
            return room->foods[i];
        }
    }
}

int isTrap(Room *room, Point p)
{
    if (room == NULL)
        return -1;
    for (int i = 0; i < room->trapCount; i++)
    {
        if (room->traps[i].cord.x == p.x && room->traps[i].cord.y == p.y && !room->traps[i].isVisible)
        {
            return i;
        }
    }
    return -1;
}

int trapMode = 0;
void movePlayer(Player *player, Room **rooms, Passway **passways, int roomsCount, int x, int y)
{
    char c = mvinch(y, x);
    Point cur;
    cur.x = x;
    cur.y = y;
    if (trapMode == 1)
    {
        mvprintw(player->cord.y, player->cord.x, "^");
        refresh();
        // trapMode = 0;
    }
    if (c == '@')
    {
        if (player->room->doors[0].password)
        {
            clear();
            refresh();
            int check = 0;
            for (int i = 0; i < 3; i++)
            {
                WINDOW *formWin = creaetMenuWindow(20, maxX / 3 + 5, maxY / 2 - 15, maxX / 6 + 15);
                echo();
                refresh();
                char **headers = malloc(sizeof(char *));
                headers[0] = "Password";
                wmove(formWin, 3, maxX / 2 - 15);
                refresh();
                char **result = handleInput(formWin, 1, headers, maxY / 2 - 15, maxX / 6 + 15);
                int password = stringToNumber(result[0]);
                if (player->room->doors[0].password != password)
                {
                    if (i == 0)
                    {
                        wattron(formWin, COLOR_PAIR(3));
                    }
                    else if (i == 1)
                    {
                        wattron(formWin, COLOR_PAIR(4));
                    }
                    mvwprintw(formWin, 14, 4, "Password is wrong %d try", i + 1);
                    wrefresh(formWin);
                    if (i == 0)
                    {
                        wattroff(formWin, COLOR_PAIR(3));
                    }
                    else if (i == 1)
                    {
                        wattroff(formWin, COLOR_PAIR(4));
                    }
                    wrefresh(formWin);
                    getchar();
                    wclear(formWin);
                    clear();
                    free(headers);
                    refresh();
                }
                else
                {
                    wclear(formWin);
                    free(headers);
                    clear();
                    refresh();
                    check = 1;
                    break;
                }
            }
            if (check)
            {
                noecho();
                curs_set(false);
                showLevel(game->levels[game->currentLevel]);
                player->room->doors[0].password = 0;
                player->room->doors[1].password = 0;
                player->room->doors[1].type = 'n';
                player->room->doors[0].type = 'n';
                c = '+';
                mvprintw(player->room->cord.y + 3, player->room->cord.x + 1, ".");
                refresh();
            }
            else
            {
                clear();
                player->room->doors[0].password = 0;
                player->room->doors[1].password = 0;
                showLevel(game->levels[game->currentLevel]);
                refresh();
                curs_set(false);
                mvprintw(1, 1, "You are imprisoned for 5 seconds !!");
                refresh();
                sleep(5);
                mvprintw(1, 1, "                                     ");
                refresh();
            }
        }
    }
    if (c == '&')
    {
        int pass = randomNumber(1000, 9999);
        int num = rand();
        if (num % 5 == 2)
        {
            player->room->doors[0].password = reverseNumber(pass);
            player->room->doors[1].password = reverseNumber(pass);
        }
        else
        {
            player->room->doors[0].password = pass;
            player->room->doors[1].password = pass;
        }
        mvprintw(1, 1, "password : %d", pass);
        refresh();
        sleep(5);
        mvprintw(1, 1, "                       ");
        refresh();
    }
    if (c == '>')
    {
        Stair stair = player->room->stair;
        if (changeLevel(stair))
        {
            trapMode = 0;
            return;
        }
    }
    if (c == '+')
    {
        if (player->passway == NULL)
        {
            player->passway = passways[0];
            player->passway->visiblePoint = 4;
            showPass(player->passway);
            player->passway->visiblePoint += 2;
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
                rooms[player->room->index + 1]->isVisible = true;
                printRoom(rooms[player->room->index + 1]);
            }
        }
    }
    if (c == '$')
    {
        if (player->foodCount < 5)
        {
            Food f = findFood(cur, player->room);
            player->foods[player->foodCount] = (Food *)malloc(sizeof(Food));
            player->foods[player->foodCount]->cord = f.cord;
            player->foods[player->foodCount]->health = f.health;
            player->foods[player->foodCount]->isUsed = f.isUsed;
            player->foodCount++;
            c = '.';
            if (player->room->foodCount)
            {
                player->room->foodCount -= 1;
            }
            showPlayeInfo(*player);
        }
    }
    if ((c == '.' || c == '+' || c == '#') && x >= 0 && x <= maxX && y > 3 && y <= maxY)
    {
        if (!trapMode)
        {
            mvprintw(player->cord.y, player->cord.x, c == '.' ? "." : c == '+' ? "."
                                                                               : "#");
        }
        else
        {
            trapMode = 0;
        }
        player->cord.x = x;
        player->cord.y = y;
        mvprintw(player->cord.y, player->cord.x, "@");
        refresh();
    }
    int trapIndex = isTrap(player->room, cur);
    if (trapIndex != -1)
    {
        player->room->traps[trapIndex].isVisible = true;
        player->health -= 3;
        trapMode = 1;
        showPlayeInfo(*player);
    }
    int index = inRoom(rooms, roomsCount, player->cord);
    if (index != -1)
    {
        player->room = rooms[index];
        rooms[index]->isVisible = true;
        player->state = 1;
    }
}

void printFoods(Room *room)
{
    for (int i = 0; i < room->foodCount; i++)
    {
        if (!room->foods[i].isUsed)
            mvprintw(room->foods[i].cord.y, room->foods[i].cord.x, "$");
    }
    refresh();
}
void printTraps(Room *room)
{
    for (int i = 0; i < room->trapCount; i++)
    {
        if (room->traps[i].isVisible)
            mvprintw(room->traps[i].cord.y, room->traps[i].cord.x, "^");
    }
    refresh();
}
void printStair(Room *room)
{
    if (room->stairCount)
        mvprintw(room->stair.cord.y, room->stair.cord.x, ">");
    refresh();
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
    printFoods(room);
    printTraps(room);
    printStair(room);
    refresh();
}