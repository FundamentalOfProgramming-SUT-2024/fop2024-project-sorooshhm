#include <ncurses.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <locale.h>
#include <SDL2/SDL_mixer.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "auth.h"
#include "utils.h"
#include "game.h"
#include "menu.h"
#include "form.h"
#include "db.h"

#define MAX_HEIGHT 12
#define MIN_HEIGHT 10
#define MAX_WIDTH 16
#define MIN_WIDTH 10

void handleMove();
void moveEnemies(Player *player);
void movePlayer(Player *player, Room **rooms, Passway **passways, int roomsCount, int x, int y);
Room *createRoom(Room **rooms, int roomsCount, int min_x, int min_y, int max_X, int max_y);
bool hasOverlap(Room a, Room b);
void printDoors(Room *room);
void printRoom(Room *room);
void printJustRooms(Room *room);
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

int enchantMode = 0;
int healthEnchant = 0;
int speedEnchant = 0;
int enchantMove = 0;
int speed = 1;
int mapMode = 0;
volatile int damageTime = 30;
int win_state = 0;
WINDOW *mapWin;
Game *game;
Player *player;
Gun *curGun;
User *u;
Enemy *curEnemy = NULL;
int marked_enemies[100] = {0};
long long milliseconds;

void freeArr()
{
    for (int i = 0; i < 100; i++)
    {
        marked_enemies[i] = 0;
    }
}

void startGame(User *user, Mix_Music *music)
{

    freeArr();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    milliseconds = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
    u = user;
    srand(time(NULL));
    // printf("%d , %d\n", maxX, maxY);
    noecho();
    damageTime -= (user->setting.level - 1) * 5;

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
    player->health = 100;
    player->foodCount = 0;
    player->gunCount = 5;
    player->enchantCount = 3;
    player->power = 1;
    player->guns = (Gun **)malloc(100 * sizeof(Gun *));
    // initialize guns
    {
        player->guns[0] = (Gun *)malloc(sizeof(Gun));
        player->guns[0]->count = 1;
        player->guns[0]->type = 'g';
        player->guns[0]->damage = 5;
        player->guns[0]->isUsed = true;
        player->guns[1] = (Gun *)malloc(sizeof(Gun));
        player->guns[1]->count = 0;
        player->guns[1]->type = 'k';
        player->guns[1]->damage = 12;
        player->guns[1]->isUsed = true;
        player->guns[2] = (Gun *)malloc(sizeof(Gun));
        player->guns[2]->count = 0;
        player->guns[2]->type = 'a';
        player->guns[2]->damage = 15;
        player->guns[2]->isUsed = true;
        player->guns[3] = (Gun *)malloc(sizeof(Gun));
        player->guns[3]->count = 0;
        player->guns[3]->type = 't';
        player->guns[3]->damage = 5;
        player->guns[3]->isUsed = true;
        player->guns[4] = (Gun *)malloc(sizeof(Gun));
        player->guns[4]->count = 0;
        player->guns[4]->type = 's';
        player->guns[4]->damage = 10;
        player->guns[4]->isUsed = true;
    }
    // initilize encants
    {
        player->enchants = (Enchant **)malloc(100 * sizeof(Enchant *));
        player->enchants[0] = (Enchant *)malloc(sizeof(Enchant));
        player->enchants[0]->type = 'h';
        player->enchants[0]->count = 0;
        player->enchants[1] = (Enchant *)malloc(sizeof(Enchant));
        player->enchants[1]->type = 's';
        player->enchants[1]->count = 0;
        player->enchants[2] = (Enchant *)malloc(sizeof(Enchant));
        player->enchants[2]->type = 'd';
        player->enchants[2]->count = 0;
    }
    player->level = 0;
    player->acientKey = 0;
    player->brokenAcientKey = 0;
    player->gold = 0;
    player->name = user->username;
    player->enemyCount = 0;
    player->enemies = (Enemy **)malloc(sizeof(Enemy *) * 10);
    game->player = player;
    curGun = player->guns[0];
    showLevel(game->levels[game->currentLevel]);

    keypad(stdscr, true);

    // damage the player
    pthread_t damageThread;
    pthread_create(&damageThread, NULL, damagePlayer, NULL);
    // pthread_join(damageThread, NULL);

    // checking key pressing
    handleMove(game->levels[game->currentLevel]);

    pthread_cancel(damageThread);
    // saving the game
    if (u->isAuth)
    {
        player->enemyCount = 0;
        if (player->enemies)
        {
            player->enemies[0] = NULL;
        }
        saveGame(game, user);
    }

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
        if (p.x == passways[i]->points[0].x && (p.y == passways[i]->points[0].y + 1 || p.y == passways[i]->points[0].y - 1))
        {
            passways[i]->visiblePoint = 5;
            return i;
        }
    }
    return -1;
}

void resumeGame(User *user, Mix_Music *music)
{
    freeArr();

    struct timeval tv;
    gettimeofday(&tv, NULL);
    milliseconds = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
    srand(time(NULL));
    noecho();
    damageTime -= (user->setting.level - 1) * 5;

    u = user;
    game = (Game *)malloc(sizeof(Game));
    int check = loadGame(game, user);
    if (!check)
    {
        return;
    }
    for (int i = game->currentLevel + 1; i < 4; i++)
    {
        game->levels[i] = (Level *)malloc(sizeof(Level));
        createLevel(game->levels[i], i);
    }

    player = game->player;
    player->passway = NULL;
    player->room = NULL;
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
    if (!game->player->gunCount)
    {
        game->player->guns = (Gun **)malloc(100 * sizeof(Gun));
    }
    if (!game->player->enchantCount)
    {
        game->player->enchants = (Enchant **)malloc(100 * sizeof(Enchant *));
    }

    player->enemyCount = 0;
    player->enemies = (Enemy **)malloc(sizeof(Enemy *) * 10);
    curGun = player->guns[0];
    player->power = 1;

    showLevel(game->levels[game->currentLevel]);

    keypad(stdscr, true);

    // damage the player
    pthread_t damageThread;
    pthread_create(&damageThread, NULL, damagePlayer, NULL);
    // pthread_join(damageThread, NULL);

    // checking key pressing
    handleMove(game->levels[game->currentLevel]);

    pthread_cancel(damageThread);
    // saving the game
    if (u->isAuth)
    {
        player->enemyCount = 0;
        if (player->enemies)
        {
            player->enemies[0] = NULL;
        }
        saveGame(game, user);
    }
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < game->levels[i]->roomsCount; j++)
        {
            free(game->levels[i]->rooms[j]);
        }
        for (int j = 0; j < game->levels[i]->roomsCount - 1; j++)
        {
            free(game->levels[i]->passways[j]);
        }
        free(game->levels[i]);
    }
    free(game);
    clear();
    refresh();
    // breaking the game
    Mix_FreeMusic(music);
    // pthread_join(damageThread, NULL);
    return;
}

void win()
{
    if (u->isAuth)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        u->golds += player->gold;
        u->score += (player->gold * u->setting.level) / 5;
        u->games += 1;
        long long newmil = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
        u->gameplay += (newmil - milliseconds) / 1000;
    }

    WINDOW *win = newwin(maxY, maxX, 0, 0);
    mvwprintw(win, maxY / 2, 50, "You won !!!! %lc", L'😍');
    mvwprintw(win, maxY / 2 + 2, 50, "Your golds : %d Your score : %d", player->gold, (player->gold * u->setting.level) / 5);
    wrefresh(win);
    sleep(5);
    wclear(win);
    clear();
    // char *filename = "./games/";
    // strcat(filename, u->username);
    // remove(filename);
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
    if (player->enemyCount)
    {
        player->enemyCount = 0;
        marked_enemies[player->enemies[0]->id] = 0;
        player->enemies[0] = NULL;
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
        if (i == roomsCounts - 1)
        {
            rooms[i]->doors[0].type = 'n';
            rooms[i]->doors[1].type = 'n';
        }
        rooms[i]->index = i;
        rooms[i]->keyCount = 0;
        if (levelIndex >= 2)
        {
            int randomNum = rand();
            if (randomNum % 6 == 1)
            {
                rooms[i]->doors[0].type = 'h';
                rooms[i]->doors[1].type = 'h';
                rooms[i]->doors[0].password = 0;
                rooms[i]->doors[1].password = 0;
            }
        }
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
    int id = 1;
    // adding foods , traps , stairs
    for (int i = 0; i < roomsCounts; i++)
    {
        rooms[i]->foodCount = 0;
        rooms[i]->trapCount = 0;
        rooms[i]->stairCount = 0;
        int rndm = rand();
        if (levelIndex == 3 && i == roomsCounts - 1)
        {
            rooms[i]->foodCount = 0;
            int count = 6;
            rooms[i]->trapCount = count;
            rooms[i]->traps = (Trap *)malloc(count * sizeof(Trap));
            for (int j = 0; j < count; j++)
            {
                rooms[i]->traps[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                rooms[i]->traps[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                rooms[i]->traps[j].isVisible = false;
            }
            rooms[i]->type = 'g';
        }
        else
        {
            // normal room
            if (rndm % 8 <= 4)
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
                count = randomNumber(1, 3);
                rooms[i]->trapCount = count;
                rooms[i]->traps = (Trap *)malloc(count * sizeof(Trap));
                for (int j = 0; j < count; j++)
                {
                    rooms[i]->traps[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                    rooms[i]->traps[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                    rooms[i]->traps[j].isVisible = false;
                }
                count = randomNumber(0, 2);
                rooms[i]->goldCount = count;
                rooms[i]->golds = (Gold *)malloc(count * sizeof(Gold));
                for (int j = 0; j < count; j++)
                {
                    int num = rand();
                    rooms[i]->golds[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                    rooms[i]->golds[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                    rooms[i]->golds[j].isUsed = false;
                    if (num % 8 == 1)
                    {
                        rooms[i]->golds[j].type = 'b';
                        rooms[i]->golds[j].count = 10;
                    }
                    else
                    {
                        rooms[i]->golds[j].type = 'g';
                        rooms[i]->golds[j].count = 5;
                    }
                }
                count = randomNumber(0, 1);
                rooms[i]->gunCount = count;
                rooms[i]->guns = (Gun *)malloc(100 * sizeof(Gun));
                for (int j = 0; j < count; j++)
                {
                    int num = rand();
                    rooms[i]->guns[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 3);
                    rooms[i]->guns[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 4);
                    rooms[i]->guns[j].isUsed = false;
                    if ((num % 4 == 3))
                    {
                        rooms[i]->guns[j].type = 'k';  // khanjar
                        rooms[i]->guns[j].damage = 12; // khanjar
                        rooms[i]->guns[j].count = 10;
                    }
                    else if (num % 4 == 2)
                    {
                        rooms[i]->guns[j].type = 'a';  // asa
                        rooms[i]->guns[j].damage = 15; // khanjar
                        rooms[i]->guns[j].count = 8;
                    }
                    else if (num % 4 == 1)
                    {
                        rooms[i]->guns[j].type = 't'; // tir
                        rooms[i]->guns[j].damage = 5; // khanjar
                        rooms[i]->guns[j].count = 20;
                    }
                    else
                    {
                        rooms[i]->guns[j].type = 's';  // shamshir
                        rooms[i]->guns[j].damage = 10; // khanjar
                        rooms[i]->guns[j].count = 1;
                    }
                }
                count = randomNumber(0, 1);
                rooms[i]->enchantCount = count;
                rooms[i]->enchants = (Enchant *)malloc(count * sizeof(Enchant));
                for (int j = 0; j < count; j++)
                {
                    int num = rand();
                    rooms[i]->enchants[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 3);
                    rooms[i]->enchants[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 4);
                    rooms[i]->enchants[j].isUsed = false;
                    if (num % 3 == 2)
                    {
                        rooms[i]->enchants[j].type = 'h'; // health
                    }
                    else if (num % 3 == 1)
                    {
                        rooms[i]->enchants[j].type = 's'; // speed
                    }
                    else
                    {
                        rooms[i]->enchants[j].type = 'd'; // damage
                    }
                }

                rooms[i]->type = 'n';
            }
            // enchant room
            else if (((i == 0 && levelIndex != 0) || i == roomsCounts - 1) && rndm % 8 > 4)
            {
                rooms[i]->foodCount = 0;
                int count = randomNumber(1, 3);
                rooms[i]->trapCount = count;
                rooms[i]->traps = (Trap *)malloc(count * sizeof(Trap));
                for (int j = 0; j < count; j++)
                {
                    rooms[i]->traps[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                    rooms[i]->traps[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                    rooms[i]->traps[j].isVisible = false;
                }
                count = randomNumber(2, 6);
                rooms[i]->enchantCount = count;
                rooms[i]->enchants = (Enchant *)malloc(count * sizeof(Enchant));
                for (int j = 0; j < count; j++)
                {
                    int num = rand();
                    rooms[i]->enchants[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 3);
                    rooms[i]->enchants[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 4);
                    rooms[i]->enchants[j].isUsed = false;
                    if (num % 3 == 2)
                    {
                        rooms[i]->enchants[j].type = 'h'; // health
                    }
                    else if (num % 3 == 1)
                    {
                        rooms[i]->enchants[j].type = 's'; // speed
                    }
                    else
                    {
                        rooms[i]->enchants[j].type = 'd'; // damage
                    }
                }
                if (i == 0)
                {
                    rooms[i]->doors[0].type = 's';
                    rooms[i]->doors[0].isVisible = false;
                    // rooms[i + 1]->doors[0].type = 's';
                    // rooms[i + 1]->doors[0].isVisible = false;
                }
                else if (i == roomsCounts - 1)
                {
                    rooms[i]->doors[0].type = 's';
                    rooms[i]->doors[0].isVisible = false;
                    if (roomsCounts == 6)
                    {
                        rooms[i - 1]->doors[1].type = 's';
                        rooms[i - 1]->doors[1].isVisible = false;
                    }
                    else
                    {
                        rooms[i - 1]->doors[0].type = 's';
                        rooms[i - 1]->doors[0].isVisible = false;
                    }
                }
                rooms[i]->type = 't';
            }
            // another normal room
            else
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
                count = randomNumber(1, 3);
                rooms[i]->trapCount = count;
                rooms[i]->traps = (Trap *)malloc(count * sizeof(Trap));
                for (int j = 0; j < count; j++)
                {
                    rooms[i]->traps[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                    rooms[i]->traps[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                    rooms[i]->traps[j].isVisible = false;
                }
                count = randomNumber(0, 2);
                rooms[i]->goldCount = count;
                rooms[i]->golds = (Gold *)malloc(count * sizeof(Gold));
                for (int j = 0; j < count; j++)
                {
                    int num = rand();
                    rooms[i]->golds[j].cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 2);
                    rooms[i]->golds[j].cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 3);
                    rooms[i]->golds[j].isUsed = false;
                    if (num % 8 == 1)
                    {
                        rooms[i]->golds[j].type = 'b';
                        rooms[i]->golds[j].count = 10;
                    }
                    else
                    {
                        rooms[i]->golds[j].type = 'g';
                        rooms[i]->golds[j].count = 5;
                    }
                }
                rooms[i]->type = 'n';
            }
        }
        int num = rand();
        if (num)
        {
            rooms[i]->enemyCount = 1;

            if (num % 5 == 0)
            {
                Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
                enemy->type = 'D';
                enemy->health = 5;
                enemy->damage = 1;
                rooms[i]->enemy = enemy;
            }
            else if (num % 5 == 1)
            {
                Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
                enemy->type = 'F';
                enemy->health = 10;
                enemy->damage = 1;
                rooms[i]->enemy = enemy;
            }
            else if (num % 5 == 2)
            {
                Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
                enemy->type = 'G';
                enemy->health = 15;
                enemy->damage = 1;
                rooms[i]->enemy = enemy;
            }
            else if (num % 5 == 3)
            {
                Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
                enemy->type = 'S';
                enemy->health = 20;
                enemy->damage = 1;
                rooms[i]->enemy = enemy;
            }
            else if (num % 5 == 4)
            {
                Enemy *enemy = (Enemy *)malloc(sizeof(Enemy));
                enemy->type = 'U';
                enemy->health = 30;
                enemy->damage = 1;
                rooms[i]->enemy = enemy;
            }
            rooms[i]->enemy->cord.x = randomNumber(rooms[i]->cord.x + 2, rooms[i]->cord.x + rooms[i]->width - 3);
            rooms[i]->enemy->cord.y = randomNumber(rooms[i]->cord.y + 3, rooms[i]->cord.y + rooms[i]->height - 4);
            rooms[i]->enemy->isVisible = false;
            rooms[i]->enemy->id = id++;
            rooms[i]->enemy->moves = 0;
            rooms[i]->enemy->canMove = true;
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

    int roomIndex = rand() % roomsCounts;
    rooms[roomIndex]->keyCount = 1;
    level->key.x = rooms[roomIndex]->cord.x + rooms[roomIndex]->width - 2;
    level->key.y = rooms[roomIndex]->cord.y + rooms[roomIndex]->height - 3;

    level->level = levelIndex;
    level->rooms = rooms;
    level->passways = passways;
    level->roomsCount = roomsCounts;
}

void lose()
{
    if (u->isAuth)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        u->golds += player->gold;
        u->score += (player->gold * u->setting.level) / 5;
        long long newmil = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
        u->gameplay += (newmil - milliseconds) / 1000;
    }

    WINDOW *lostWin = newwin(maxY, maxX, 0, 0);
    mvwprintw(lostWin, maxY / 2, 50, "RIP dear %s :(( ...", player->name);
    mvwprintw(lostWin, maxY / 2 + 2, 50, "Your golds : %d Your score : %d", player->gold, (player->gold * u->setting.level) / 5);
    wrefresh(lostWin);
    sleep(5);
    wclear(lostWin);
    clear();
}

void *damagePlayer(void *args)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    int counter = 0;
    while (1)
    {
        sleep(1);
        counter++;
        if (counter >= damageTime)
        {
            (player)->health -= 5;
            showPlayeInfo(*player);
            if (player->health <= 0)
            {
                break;
            }
            counter = 0;
        }
    }
    pthread_exit(NULL);
}

void showPlayeInfo(Player player)
{
    mvprintw(1, maxX / 2, "                                                                                                           ");
    refresh();
    mvprintw(1, maxX / 2, "Level : %d golds : %d foods : %d health : %d key : %d brokenKey : %d ", player.level + 1, player.gold, player.foodCount, player.health, player.acientKey, player.brokenAcientKey);
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
    if (u->setting.color == 1)
    {
        attron(COLOR_PAIR(2));
    }
    else if (u->setting.color == 2)
    {
        attron(COLOR_PAIR(1));
    }
    mvprintw(game->player->cord.y, game->player->cord.x, "@");
    if (u->setting.color == 1)
    {
        attroff(COLOR_PAIR(2));
    }
    else if (u->setting.color == 2)
    {
        attroff(COLOR_PAIR(1));
    }
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
        room->doors[i].isVisible = false;
    }
    if (num % 4 == 1)
    {
        room->window.x = room->cord.x + room->width;
        room->window.y = room->cord.y + 2 + randomNumber(0, room->height - 4);
    }
    room->pillar.cord.x = 0;
    room->pillar.cord.y = 0;
    if (num % 5 <= 1)
    {
        room->pillar.cord.x = randomNumber(room->cord.x + 2, room->cord.x + room->width - 2);
        room->pillar.cord.y = randomNumber(room->cord.y + 3, room->cord.y + room->height - 3);
    }
    else
    {
        room->window.x = 0;
        room->window.y = 0;
    }
    room->foodCount = 0;
    room->trapCount = 0;
    room->goldCount = 0;
    room->enchantCount = 0;
    room->gunCount = 0;
    room->enchants = NULL;
    room->guns = NULL;
    room->enemy = NULL;
    room->enemyCount = 0;
    return room;
}

bool isInRoom(Room *room, Point p)
{
    return p.x > room->cord.x && p.x < (room->cord.x + room->width) && p.y > room->cord.y && p.y < (room->cord.y + room->height);
}

int isWay(Point p)
{
    char c = mvinch(p.y, p.x);
    if (c != '|' && c != '-' && c != '_' && c != '@' && c != ' ' && c != '&' && c != '=' && !(p.x == player->cord.x && p.y == player->cord.y))
    {
        return 1;
    }
    return 0;
}

void showEnemy(Enemy *enemy)
{
    attron(COLOR_PAIR(4));
    mvprintw(enemy->cord.y, enemy->cord.x, "%c", enemy->type);
    attroff(COLOR_PAIR(4));
    refresh();
}

void moveEnemies(Player *player)
{
    for (int i = 0; i < player->enemyCount; i++)
    {
        Enemy *enemy = player->enemies[i];
        Point p;
        Point prev_p = enemy->cord;
        if (!enemy->canMove)
            continue;
        int move_x = enemy->cord.x > player->cord.x ? -1 : 1;
        p.x = enemy->cord.x + move_x;
        p.y = enemy->cord.y;
        if (isWay(p))
        {
            enemy->cord = p;
        }
        int move_y = enemy->cord.y > player->cord.y ? -1 : 1;
        p.y = enemy->cord.y + move_y;
        p.x = enemy->cord.x;
        if (isWay(p))
        {
            enemy->cord = p;
        }
        enemy->moves++;
        if ((enemy->type == 'G' || enemy->type == 'U') && enemy->moves >= 5)
        {
            player->enemyCount = 0;
            player->enemies[0] = NULL;
            marked_enemies[enemy->id] = 0;
            enemy->moves = 0;
            return;
        }
        if (mvinch(prev_p.y, prev_p.x - 1) == '-' || mvinch(prev_p.y, prev_p.x + 1) == '-')
        {
            mvprintw(prev_p.y, prev_p.x, "+");
        }
        else if (inPassway(game->levels[game->currentLevel]->passways, game->levels[game->currentLevel]->roomsCount - 1, prev_p) == -1)
        {
            mvprintw(prev_p.y, prev_p.x, ".");
        }
        else
        {

            mvprintw(prev_p.y, prev_p.x, "#");
        }
        showEnemy(enemy);
    }
}

int isNear(Point p, Point c)
{
    if (p.x - 1 == c.x && p.y == c.y)
        return 1;
    if (p.x + 1 == c.x && p.y == c.y)
        return 1;
    if (p.x == c.x && p.y + 1 == c.y)
        return 1;
    if (p.x == c.x && p.y - 1 == c.y)
        return 1;
    if (p.x - 1 == c.x && p.y + 1 == c.y)
        return 1;
    if (p.x - 1 == c.x && p.y - 1 == c.y)
        return 1;
    if (p.x + 1 == c.x && p.y - 1 == c.y)
        return 1;
    if (p.x + 1 == c.x && p.y + 1 == c.y)
        return 1;
    if (p.x == c.x && p.y == c.y)
        return 1;
    return 0;
}

void doEnchants(char type)
{
    if (type == 'h')
    {
        enchantMode = 1;
        damageTime += 5;
        healthEnchant = 1;
    }
    else if (type == 's')
    {
        // TODO
    }
    else if (type == 'd')
    {
        player->power = 2;
        enchantMode = 1;
    }
}

int isPlayerWay(int x, int y)
{
    char c = mvinch(y, x);
    if ((c == '.' || c == '+' || c == '#' || c == '?') && x >= 0 && x <= maxX && y > 3 && y <= maxY)
    {
        return 1;
    }
    return 0;
}

void handleMove()
{
    int check = 1;
    while (1)
    {
        Level *level = game->levels[game->currentLevel];
        if (win_state)
        {
            win();
            break;
        }
        if (player->health <= 0)
        {
            lose();
            break;
        }
        char c = getchar();
        if (enchantMode)
        {
            enchantMove++;
        }
        if (enchantMove >= 25)
        {
            enchantMode = 0;
            enchantMove = 0;
            if (healthEnchant)
            {
                damageTime -= 5;
            }
            if (speedEnchant)
            {
                speedEnchant = 0;
            }
        }
        if (c == 27)
        {
            break;
        }
        else if (c == ' ')
        {
            if (player->enemyCount)
            {
                Enemy *enemy = player->enemies[0];

                if (curGun->count == 0)
                {
                    mvprintw(1, 1, "You dont have enough gun");
                    refresh();
                    getchar();
                    mvprintw(1, 1, "                              ");
                    refresh();
                    continue;
                }
                if (curGun->type == 'g')
                {
                    if (isNear(player->cord, enemy->cord))
                    {
                        enemy->health -= curGun->damage * player->power;
                        mvprintw(1, 1, "Cool ! you hit the enemy ");
                        refresh();
                        // message for hitting enemy
                        if (enemy->health <= 0)
                        {
                            mvprintw(1, 1, "You killed him !!!          ");
                            refresh();
                            player->enemyCount = 0;
                            player->enemies[0] = NULL;
                            enemy->isVisible = false;
                            char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                            if (c == ' ')
                            {
                                mvprintw(enemy->cord.y, enemy->cord.x, "#");
                            }
                            else
                            {
                                mvprintw(enemy->cord.y, enemy->cord.x, ".");
                            }
                            refresh();
                        }
                    }
                    else
                    {
                        mvprintw(1, 1, "Oh :( You missed to hit  ");
                        refresh();
                    }
                }
                else if (curGun->type == 'k')
                {
                    mvprintw(1, 1, "Choose a diretion");
                    refresh();
                    char ch = getchar();
                    if (ch == 's')
                    {
                        if (enemy->cord.x == player->cord.x && enemy->cord.y <= player->cord.y + 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y + 1;
                            player->room->gunCount++;
                            printRoom(player->room);
                            refresh();
                        }
                    }
                    else if (ch == 'a')
                    {
                        if (enemy->cord.y == player->cord.y && enemy->cord.x <= player->cord.x - 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x - 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'd')
                    {
                        if (enemy->cord.y == player->cord.y && enemy->cord.x <= player->cord.x + 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x + 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'w')
                    {
                        if (enemy->cord.x == player->cord.x && enemy->cord.y <= player->cord.y - 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y - 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                }
                else if (curGun->type == 'a')
                {
                    mvprintw(1, 1, "Choose a diretion");
                    refresh();
                    char ch = getchar();
                    if (ch == 's')
                    {
                        if (enemy->cord.x == player->cord.x && enemy->cord.y <= player->cord.y + 10)
                        {
                            enemy->health -= curGun->damage * player->power;
                            enemy->canMove = false;

                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y + 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'a')
                    {
                        if (enemy->cord.y == player->cord.y && enemy->cord.x <= player->cord.x - 10)
                        {
                            enemy->health -= curGun->damage * player->power;
                            enemy->canMove = false;

                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x - 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'd')
                    {
                        if (enemy->cord.y == player->cord.y && enemy->cord.x <= player->cord.x + 10)
                        {
                            enemy->health -= curGun->damage * player->power;
                            enemy->canMove = false;

                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x + 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'w')
                    {
                        if (enemy->cord.x == player->cord.x && enemy->cord.y <= player->cord.y - 10)
                        {
                            enemy->health -= curGun->damage * player->power;
                            enemy->canMove = false;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y - 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                }
                else if (curGun->type == 't')
                {
                    mvprintw(1, 1, "Choose a diretion");
                    refresh();
                    char ch = getchar();
                    if (ch == 's')
                    {
                        if (enemy->cord.x == player->cord.x && enemy->cord.y <= player->cord.y + 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y + 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'a')
                    {
                        // TODO disable enemies
                        if (enemy->cord.y == player->cord.y && enemy->cord.x <= player->cord.x - 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x - 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'd')
                    {
                        if (enemy->cord.y == player->cord.y && enemy->cord.x <= player->cord.x + 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;

                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x + 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                    else if (ch == 'w')
                    {
                        if (enemy->cord.x == player->cord.x && enemy->cord.y <= player->cord.y - 5)
                        {
                            enemy->health -= curGun->damage * player->power;
                            mvprintw(1, 1, "Cool ! you hit the enemy ");
                            refresh();
                            // message for hitting enemy
                            if (enemy->health <= 0)
                            {
                                mvprintw(1, 1, "You killed him !!!          ");
                                refresh();
                                player->enemyCount = 0;
                                player->enemies[0] = NULL;
                                enemy->isVisible = false;
                                char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                                if (c == ' ')
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, "#");
                                }
                                else
                                {
                                    mvprintw(enemy->cord.y, enemy->cord.x, ".");
                                }
                                refresh();
                            }
                        }
                        else
                        {
                            mvprintw(1, 1, "Oh :( You missed to hit  ");
                            if (curGun)
                                curGun->count--;
                            if (player->room->guns == NULL)
                            {
                                player->room->guns = (Gun *)malloc(100 * sizeof(Gun));
                            }
                            player->room->guns[player->room->gunCount] = *curGun;
                            player->room->guns[player->room->gunCount].isUsed = false;
                            player->room->guns[player->room->gunCount].count = 1;
                            player->room->guns[player->room->gunCount].cord.x = player->cord.x;
                            player->room->guns[player->room->gunCount].cord.y = player->cord.y - 1;
                            player->room->gunCount++;
                            printRoom(player->room);

                            refresh();
                        }
                    }
                }
                else if (curGun->type == 's')
                {
                    if (isNear(player->cord, enemy->cord))
                    {
                        enemy->health -= curGun->damage * player->power;
                        mvprintw(1, 1, "Cool ! you hit the enemy ");
                        refresh();
                        // message for hitting enemy
                        if (enemy->health <= 0)
                        {
                            mvprintw(1, 1, "You killed him !!!          ");
                            refresh();
                            player->enemyCount = 0;
                            player->enemies[0] = NULL;
                            enemy->isVisible = false;
                            char c = mvinch(enemy->cord.y, enemy->cord.x - 1);
                            if (c == ' ')
                            {
                                mvprintw(enemy->cord.y, enemy->cord.x, "#");
                            }
                            else
                            {
                                mvprintw(enemy->cord.y, enemy->cord.x, ".");
                            }
                            refresh();
                        }
                    }
                    else
                    {
                        mvprintw(1, 1, "Oh :( You missed to hit  ");
                        refresh();
                    }
                }
            }
        }
        else if (c == 'w' || c == '8')
        {
            if (speedEnchant)
            {
                if (isPlayerWay(game->player->cord.x, game->player->cord.y - 2))
                {
                    movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x, game->player->cord.y - 2);
                    continue;
                }
            }
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x, game->player->cord.y - 1);
        }
        else if (c == 's' || c == '2')
        {
            if (speedEnchant)
            {
                if (isPlayerWay(game->player->cord.x, game->player->cord.y + 2))
                {
                    movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x, game->player->cord.y + 2);
                    continue;
                }
            }
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x, game->player->cord.y + 1);
        }
        else if (c == 'd' || c == '6')
        {
            if (speedEnchant)
            {
                if (isPlayerWay(game->player->cord.x + 2, game->player->cord.y))
                {
                    movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x + 2, game->player->cord.y);
                    continue;
                }
            }
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x + 1, game->player->cord.y);
        }
        else if (c == 'a' || c == '4')
        {
            if (speedEnchant)
            {
                if (isPlayerWay(game->player->cord.x - 2, game->player->cord.y))
                {
                    movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x - 2, game->player->cord.y);
                    continue;
                }
            }
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x - 1, game->player->cord.y);
        }
        if (c == 'q')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x - 1, game->player->cord.y - 1);
        }
        if (c == 'r')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x + 1, game->player->cord.y - 1);
        }
        if (c == 'z')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x - 1, game->player->cord.y + 1);
        }
        if (c == 'c')
        {
            movePlayer(game->player, level->rooms, level->passways, level->roomsCount, game->player->cord.x + 1, game->player->cord.y + 1);
        }
        else if (c == 'm')
        {

            mapWin = newwin(maxY, maxX, 0, 0);
            mvwprintw(mapWin, 1, 1, "Map Mode !!");
            wrefresh(mapWin);
            mapMode = 1;
            for (int i = 0; i < level->roomsCount; i++)
            {
                printJustRooms(level->rooms[i]);
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
            player->foodCount--;
            player->health += 3;
            getchar();
            wclear(menuWin);
            free(menu);
            clear();
            refresh();
            showLevel(level);
        }
        else if (c == 't')
        {
            if (player->enchantCount == 0)
            {
                continue;
            }
            WINDOW *menuWin = creaetMenuWindow(20, maxX / 2, maxY / 2 - 15, maxX / 4);

            char **menu = (char **)malloc(player->enchantCount * sizeof(char *));
            for (int i = 0; i < player->enchantCount; i++)
            {
                // if (!player.usedFood[i])
                // {
                menu[i] = malloc(50 * sizeof(char));
                if (player->enchants[i]->type == 'h')
                {
                    sprintf(menu[i], "Helath enchant ❤️‍🩹  %d", player->enchants[i]->count);
                }
                else if (player->enchants[i]->type == 's')
                {
                    sprintf(menu[i], "Speed enchant ⚡ %d", player->enchants[i]->count);
                }
                else
                {
                    sprintf(menu[i], "Damage enchant ☠️ %d", player->enchants[i]->count);
                }
                // }
            }

            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Enchant menu");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);
            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, player->enchantCount, 0);
            if (player->enchants[highlight]->count)
            {
                player->enchants[highlight]->count--;
                enchantMode = 1;
                if (player->enchants[highlight]->type == 'h')
                {
                    healthEnchant = 1;
                }
                if (player->enchants[highlight]->type == 's')
                {
                    speedEnchant = 1;
                }
            }
            wclear(menuWin);
            free(menu);
            clear();
            refresh();
            showLevel(level);
        }
        else if (c == 'g')
        {
            if (player->gunCount == 0)
            {
                continue;
            }
            WINDOW *menuWin = creaetMenuWindow(25, maxX / 2, maxY / 2 - 15, maxX / 4);

            char **menu = (char **)malloc(player->gunCount * sizeof(char *));
            for (int i = 0; i < 5; i++)
            {
                // if (!player.usedFood[i])
                // {
                menu[i] = malloc(100 * sizeof(char));
                if (player->guns[i]->type == 'g')
                {
                    sprintf(menu[i], "Gorz ⚒  %d", player->guns[i]->count);
                }
                else if (player->guns[i]->type == 'k')
                {
                    sprintf(menu[i], "Khanjar 🗡️  %d", player->guns[i]->count);
                }
                else if (player->guns[i]->type == 'a')
                {
                    sprintf(menu[i], "Asa jadooyi 🪄  %d", player->guns[i]->count);
                }
                else if (player->guns[i]->type == 't')
                {
                    sprintf(menu[i], "Tir ➳  %d", player->guns[i]->count);
                }
                else
                {
                    sprintf(menu[i], "Shamshir ⚔️  %d", player->guns[i]->count);
                }
                if (curGun->type == player->guns[i]->type)
                {
                    strcat(menu[i], "   ✅");
                }
                // }
            }

            wattron(menuWin, A_BLINK);
            mvwprintw(menuWin, 1, 25, "Gun menu");
            wrefresh(menuWin);
            wattroff(menuWin, A_BLINK);

            mvwprintw(menuWin, 2, 1, "----------------------------------------------------------------------");
            wmove(menuWin, 3, 25);
            wrefresh(menuWin);
            int highlight = handleMenuSelection(menuWin, menu, player->gunCount, 0);
            if (player->guns[highlight]->count > 0)
            {
                curGun = player->guns[highlight];
                mvprintw(1, 1, "Your current gun is %c", curGun->type);
                refresh();
                getchar();
                mvprintw(1, 1, "                               ");
                refresh();
            }
            wclear(menuWin);
            free(menu);
            clear();
            refresh();
            showLevel(level);
        }
        else if (c == 'f')
        {
            win_state = 1;
        }
        // if ((c == 'w' || c == 'a' || c == 'd' || c == 's'))
        // {
        moveEnemies(game->player);
        // }
        if (player->enemyCount)
        {
            player->health -= player->enemies[0]->damage;
            showPlayeInfo(*player);
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
        if (room->foods[i].cord.x == p.x && room->foods[i].cord.y == p.y && !room->foods[i].isUsed)
        {
            room->foods[i].isUsed = true;
            return room->foods[i];
        }
    }
}
Enchant findEnchant(Point p, Room *room)
{
    for (int i = 0; i < room->enchantCount; i++)
    {
        if (room->enchants[i].cord.x == p.x && room->enchants[i].cord.y == p.y && !room->enchants[i].isUsed)
        {
            room->enchants[i].isUsed = true;
            return room->enchants[i];
        }
    }
}
int findGun(Point p, Room *room)
{
    for (int i = 0; i < room->gunCount; i++)
    {
        if (room->guns[i].cord.x == p.x && room->guns[i].cord.y == p.y && !room->guns[i].isUsed)
        {
            return i;
        }
    }
    return -1;
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
int isGold(Room *room, Point p)
{
    for (int i = 0; i < room->goldCount; i++)
    {
        if (room->golds[i].cord.x == p.x && room->golds[i].cord.y == p.y)
        {
            return i;
        }
    }
    return -1;
}
int neighborRoom(Room **rooms, int index, int count, int y)
{
    for (int i = 0; i < count; i++)
    {
        if (y > rooms[i]->cord.y && y < rooms[i]->cord.y + rooms[i]->height && rooms[i]->index > index)
        {
            return i;
        }
    }
    return -1;
}
int isDoor(Room *room, Point p)
{
    for (int i = 0; i < room->doorCount; i++)
    {
        if ((room->doors[i].cord.x == p.x && room->doors[i].cord.y == p.y) && room->doors[i].type == 's')
        {
            return i;
        }
    }
    return -1;
}
int findPoint(Passway *passway, Point p)
{
    for (int i = 0; i < passway->count; i++)
    {
        if (passway->points[i].x == p.x && passway->points[i].y == p.y)
        {
            return i;
        }
    }
    return -1;
}
int findPassway(Passway **passways, int count, Point p)
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
int nextToEnemy(Room *room, Point p)
{
    if (room->enemyCount == 0)
    {
        return 0;
    }
    if (room->enemy == NULL)
    {
        return 0;
    }
    // if (room->enemy->health <= 0)
    // {
    //     return 0;
    // }
    if (player->enemyCount > 0)
        return 0;
    Enemy *enemy = room->enemy;
    if (enemy->cord.x == p.x || enemy->cord.y == p.y)
    {
        if (!marked_enemies[enemy->id])
        {
            curEnemy = enemy;
            marked_enemies[enemy->id] = 1;
            return 1;
        }
        return 0;
    }
    return 0;
}

int trapMode = 0;
long long doorDelay = 0;
int isEnemy(char c)
{
    return (c == 'D' || c == 'G' || c == 'F' || c == 'S' || c == 'U');
}
void movePlayer(Player *player, Room **rooms, Passway **passways, int roomsCount, int x, int y)
{
    if (game->currentLevel == 3 && player->room->index == game->levels[game->currentLevel]->roomsCount - 1)
    {
        win_state = 1;
        return;
    }
    char c = mvinch(y, x);
    refresh();
    Point cur;
    cur.x = x;
    cur.y = y;
    if (nextToEnemy(player->room, cur))
    {
        curEnemy->isVisible = true;
        player->enemies[player->enemyCount++] = curEnemy;
        curEnemy = NULL;
        mvprintw(1, 1, "An enemy is following you !");
        refresh();
        printRoom(player->room);
    }
    if (c == 'O')
    {
        return;
    }
    int doorIndex = isDoor(player->room, cur);
    if (doorIndex != -1)
    {
        player->room->doors[doorIndex].isVisible = true;
        mvprintw(player->room->doors[doorIndex].cord.y, player->room->doors[doorIndex].cord.x, "?");
        refresh();
        c = '?';
    }
    int goldIndex = isGold(player->room, cur);
    if (goldIndex != -1)
    {
        player->room->golds[goldIndex].isUsed = true;
        player->gold += player->room->golds[goldIndex].count;
        mvprintw(1, 1, "Come on !! %d golds $_$", player->room->golds[goldIndex].count);
        refresh();
        player->room->goldCount -= 1;
        mvprintw(y, x, "..");
        refresh();
        c = '.';
        showPlayeInfo(*player);
        getchar();
        mvprintw(1, 1, "                                ");
        refresh();
    }
    if (trapMode == 1)
    {
        mvprintw(player->cord.y, player->cord.x, "^");
        refresh();
        // trapMode = 0;
    }

    if (c == '=')
    {
        int roomIndex = neighborRoom(rooms, player->room->index, roomsCount, y);
        if (roomIndex != -1)
        {
            printRoom(rooms[roomIndex]);
            getchar();
            clear();
            refresh();
            showLevel(game->levels[game->currentLevel]);
        }
    }
    if (game->levels[game->currentLevel]->key.x == x && game->levels[game->currentLevel]->key.y == y)
    {
        player->acientKey += 1;
        player->room->keyCount = 0;
        game->levels[game->currentLevel]->key.x = 0;
        game->levels[game->currentLevel]->key.y = 0;
        c = '.';
        showPlayeInfo(*player);
    }
    if (c == '@')
    {
        if (player->room->doors[0].password)
        {
            if (doorDelay)
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                long long newmil = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
                if (newmil - doorDelay > 1500)
                {
                    player->room->doors[0].password = 1111;
                    player->room->doors[1].password = 1111;
                }
            }
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
                    else
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
                if (player->room->doors[0].type == 'h')
                {
                    clear();
                    refresh();
                    showLevel(game->levels[game->currentLevel]);
                }
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
                if (player->room->doors[0].type == 'h')
                {
                    clear();
                    refresh();
                    showLevel(game->levels[game->currentLevel]);
                }
            }
            doorDelay = 0;
        }
        else if (player->acientKey > 0)
        {
            mvprintw(1, 1, "You can open the door using ancient key !! (press o)");
            refresh();
            char ch = getchar();
            mvprintw(1, 1, "                                                     ");
            refresh();
            if (ch == 'o')
            {
                int num = rand();
                if (num % 10 != 8)
                {
                    player->room->doors[0].password = 0;
                    player->room->doors[1].password = 0;
                    player->room->doors[1].type = 'n';
                    player->room->doors[0].type = 'n';
                    c = '+';
                    player->acientKey -= 1;
                    mvprintw(player->room->cord.y + 3, player->room->cord.x + 1, ".");
                    refresh();
                    showPlayeInfo(*player);
                }
                else
                {
                    mvprintw(1, 1, "Oops , the key is broken now :(");
                    refresh();
                    player->acientKey -= 1;
                    player->brokenAcientKey += 1;
                    if (player->brokenAcientKey == 2)
                    {
                        player->brokenAcientKey -= 2;
                        player->acientKey += 1;
                    }
                    getchar();
                    mvprintw(1, 1, "                                  ");
                    if (player->room->doors[0].type == 'h')
                    {
                        clear();
                        refresh();
                        showLevel(game->levels[game->currentLevel]);
                    }
                    showPlayeInfo(*player);
                }
            }
            doorDelay = 0;
        }
    }
    if (c == '&')
    {
        if (player->room->doors[0].type == 'l')
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
            if (num % 8 == 1)
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                doorDelay = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
            }
            refresh();
        }
        else if (player->room->doors[0].type == 'h')
        {
            int pass = randomNumber(1000, 9999);
            int num = rand();
            if (player->room->doors[0].password)
            {
                player->room->doors[0].password += pass * 10000;
                player->room->doors[1].password += pass * 10000;
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
            mvprintw(y, x, ".");
            refresh();
        }
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
    if (c == '+' || c == '?')
    {
        player->state = 0;
        if (player->passway == NULL)
        {
            player->passway = passways[0];
            player->passway->visiblePoint = 4;
            showPass(player->passway);
        }
        else
        {
            if (player->passway->index != roomsCount - 2)
            {
                player->passway = passways[player->passway->index + 1];
                player->passway->visiblePoint = 4;
                showPass(player->passway);
            }
        }
        refresh();
    }
    if (c == '#')
    {
        int passwayIndex = findPassway(passways, roomsCount - 1, player->cord);
        if (passwayIndex != -1)
        {
            player->passway = passways[passwayIndex];
            player->state = 0;
        }
        int pIndex = findPoint(player->passway, cur);
        if (pIndex != -1)
        {
            player->passway->visiblePoint = pIndex + 5;
            showPass(player->passway);
        }
        if (player->passway->visiblePoint == player->passway->count)
        {
            if (player->room->index != roomsCount - 1)
            {
                rooms[player->passway->index + 1]->isVisible = true;
                printRoom(rooms[player->passway->index + 1]);
            }
        }
    }
    if (c == '%')
    {
        if (player->foodCount < 5)
        {
            findFood(cur, player->room);
            player->foodCount++;
            c = '.';
            showPlayeInfo(*player);
        }
    }
    if (c == 'H' || c == 'R' || c == 'I')
    {
        mvprintw(1, 1, "click <p> to pick enchant");
        refresh();
        char ch = getchar();
        mvprintw(1, 1, "                             ");
        refresh();
        if (ch == 'p')
        {
            Enchant e = findEnchant(cur, player->room);
            if (c == 'H')
            {
                player->enchants[0]->count += 1;
            }
            else if (c == 'R')
            {
                player->enchants[1]->count += 1;
            }
            else if (c == 'I')
            {
                player->enchants[2]->count += 1;
            }
            c = '.';
        }
    }
    int gunIndex = findGun(cur, player->room);
    if (gunIndex != -1)
    {
        mvprintw(1, 1, "click <p> to pick gun");
        refresh();
        char ch = getchar();
        mvprintw(1, 1, "                             ");
        refresh();
        if (ch == 'p')
        {
            Gun gun = player->room->guns[gunIndex];
            if (gun.type == 'k')
            {
                player->guns[1]->count += gun.count;
            }
            else if (gun.type == 'a')
            {
                player->guns[2]->count += gun.count;
            }
            else if (gun.type == 's')
            {
                player->guns[4]->count += gun.count;
            }
            else if (gun.type == 't')
            {
                player->guns[3]->count += gun.count;
            }
            player->room->guns[gunIndex].isUsed = true;
            c = '.';
            mvprintw(y, x, "..");
            refresh();
        }
    }
    if ((c == '.' || c == '+' || c == '#' || c == '?') && x >= 0 && x <= maxX && y > 3 && y <= maxY)
    {
        if (!trapMode)
        {
            mvprintw(player->cord.y, player->cord.x, c == '.' ? "." : (c == '+' || c == '?') ? "."
                                                                                             : "#");
        }
        else
        {
            trapMode = 0;
        }
        player->cord.x = x;
        player->cord.y = y;
        if (u->setting.color == 1)
        {
            attron(COLOR_PAIR(2));
        }
        else if (u->setting.color == 2)
        {
            attron(COLOR_PAIR(1));
        }
        mvprintw(player->cord.y, player->cord.x, "@");
        refresh();
        if (u->setting.color == 1)
        {
            attroff(COLOR_PAIR(2));
        }
        else if (u->setting.color == 2)
        {
            attroff(COLOR_PAIR(1));
        }
    }
    int trapIndex = isTrap(player->room, cur);
    if (trapIndex != -1)
    {
        player->room->traps[trapIndex].isVisible = true;
        player->health -= 3;
        trapMode = 1;
        showPlayeInfo(*player);
    }

    int passwayIndex = findPassway(passways, roomsCount - 1, player->cord);
    if (passwayIndex != -1)
    {
        player->passway = passways[passwayIndex];
        player->state = 0;
    }
    int index = inRoom(rooms, roomsCount, player->cord);
    if (index != -1)
    {
        player->room = rooms[index];
        rooms[index]->isVisible = true;
        player->state = 1;
    }
    if (player->room->type == 't' && inRoom(rooms, roomsCount, player->cord) == player->room->index)
    {
        damageTime = 5;
        showPlayeInfo(*player);
    }
    else
    {
        damageTime = 30;
    }
}

void printDoors(Room *room)
{
    WINDOW *screen = mapMode ? mapWin : stdscr;
    for (int i = 0; i < room->doorCount; i++)
    {
        if (room->doors[i].type == 'n')
        {
            mvwprintw(screen, room->doors[i].cord.y, room->doors[i].cord.x, "+");
        }
        else if (room->doors[i].type == 'l' && room->doors[1 - i].type != 's')
        {
            attron(COLOR_PAIR(1));
            mvwprintw(screen, room->doors[i].cord.y, room->doors[i].cord.x, "@");
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(2));
            mvwprintw(screen, room->cord.y + 3, room->cord.x + 1, "&");
            attroff(COLOR_PAIR(2));
        }
        else if (room->doors[i].type == 'h' && room->doors[1 - i].type != 's')
        {
            attron(COLOR_PAIR(1));
            mvwprintw(screen, room->doors[i].cord.y, room->doors[i].cord.x, "@");
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(2));
            mvwprintw(screen, room->cord.y + 3, room->cord.x + 1, "&");
            mvwprintw(screen, room->cord.y + 3, room->cord.x + room->width - 1, "&");
            attroff(COLOR_PAIR(2));
        }
        else if (room->doors[i].type == 's')
        {
            if (room->doors[i].isVisible)
                mvwprintw(screen, room->doors[i].cord.y, room->doors[i].cord.x, "?");
        }
    }
}
void printFoods(Room *room)
{
    for (int i = 0; i < room->foodCount; i++)
    {
        if (!room->foods[i].isUsed)
            mvprintw(room->foods[i].cord.y, room->foods[i].cord.x, "%%");
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
void printWindow(Room *room)
{
    if (room->window.x)
        mvprintw(room->window.y, room->window.x, "=");
    refresh();
}
void printGolds(Room *room)
{
    for (int i = 0; i < room->goldCount; i++)
    {
        if (!room->golds[i].isUsed)
        {
            if (room->golds[i].type == 'g')
                mvprintw(room->golds[i].cord.y, room->golds[i].cord.x, "%lc", L'🪙');
            else
                mvprintw(room->golds[i].cord.y, room->golds[i].cord.x, "%lc", L'💰');
        }
    }
    refresh();
}
void printGuns(Room *room)
{
    for (int i = 0; i < room->gunCount; i++)
    {
        if (!room->guns[i].isUsed)
        {
            if (room->guns[i].type == 'g')
                mvprintw(room->guns[i].cord.y, room->guns[i].cord.x, "%lc", L'⚒');
            else if (room->guns[i].type == 'k')
                mvprintw(room->guns[i].cord.y, room->guns[i].cord.x, "🗡️");
            else if (room->guns[i].type == 'a')
                mvprintw(room->guns[i].cord.y, room->guns[i].cord.x, "%lc", L'🪄');
            else if (room->guns[i].type == 't')
                mvprintw(room->guns[i].cord.y, room->guns[i].cord.x, "%lc", L'➳');
            else
                mvprintw(room->guns[i].cord.y, room->guns[i].cord.x, "⚔️");
        }
    }
    refresh();
}
void printEnchants(Room *room)
{
    for (int i = 0; i < room->enchantCount; i++)
    {
        if (!room->enchants[i].isUsed)
        {
            if (room->enchants[i].type == 'h')
                mvprintw(room->enchants[i].cord.y, room->enchants[i].cord.x, "H");
            else if (room->enchants[i].type == 's')
                mvprintw(room->enchants[i].cord.y, room->enchants[i].cord.x, "R");
            else
                mvprintw(room->enchants[i].cord.y, room->enchants[i].cord.x, "I");
        }
    }
    refresh();
}
void printPillars(Room *room)
{
    if (room->pillar.cord.x != room->pillar.cord.y != 0)
    {
        mvprintw(room->pillar.cord.y, room->pillar.cord.x, "O");
    }
}
void printEnemies(Room *room)
{
    if (room->enemy != NULL && room->enemyCount && room->enemy->isVisible && room->enemy->health > 0)
    {
        attron(COLOR_PAIR(4));
        mvprintw(room->enemy->cord.y, room->enemy->cord.x, "%c", room->enemy->type);
        attroff(COLOR_PAIR(4));
        refresh();
    }
}
void printJustRooms(Room *room)
{
    int x, y, width, height;
    x = room->cord.x;
    y = room->cord.y;
    height = room->height;
    width = room->width;
    WINDOW *screen = mapMode ? mapWin : stdscr;
    if (room->type == 't')
    {
        attron(COLOR_PAIR(5));
    }
    else if (room->type == 'g')
    {
        attron(COLOR_PAIR(3));
    }
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
    if (room->type == 't')
    {
        attroff(COLOR_PAIR(5));
    }
    else if (room->type == 'g')
    {
        attroff(COLOR_PAIR(3));
    }
    printDoors(room);
}
void printRoom(Room *room)
{
    int x, y, width, height;
    x = room->cord.x;
    y = room->cord.y;
    height = room->height;
    width = room->width;
    WINDOW *screen = mapMode ? mapWin : stdscr;
    if (room->type == 't')
    {
        attron(COLOR_PAIR(5));
    }
    else if (room->type == 'g')
    {
        attron(COLOR_PAIR(3));
    }
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
    if (room->type == 't')
    {
        attroff(COLOR_PAIR(5));
    }
    else if (room->type == 'g')
    {
        attroff(COLOR_PAIR(3));
    }
    printDoors(room);
    printFoods(room);
    printTraps(room);
    printStair(room);
    printWindow(room);
    printGolds(room);
    printGuns(room);
    printEnchants(room);
    printPillars(room);
    printEnemies(room);
    if (room->keyCount)
    {
        attron(COLOR_PAIR(3));
        mvprintw(room->cord.y + height - 3, room->cord.x + width - 2, "%lc", L'\u25B3');
        attroff(COLOR_PAIR(3));
    }
    refresh();
}
