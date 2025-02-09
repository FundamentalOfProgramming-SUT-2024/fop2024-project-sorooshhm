#ifndef HEADER_H_GAME
#define HEADER_H_GAME

#include "auth.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

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
    char type; // 'n' -> normal 's' -> secret 'l' -> locked  'h' -> hard locked
    int password;
    bool isVisible;
} Door;

typedef struct
{
    int health;
    char type;
    bool isUsed;
    Point cord;
    long long int startTime;
} Food;

typedef struct
{
    char type;
    Point cord;
    int damage;
    int health;
    int id;
    int moves;
    bool isVisible;
    bool canMove;
} Enemy;

typedef struct
{
    Point cord;
} Pillar;

typedef struct
{
    char type; // g -> gorz; k -> khanjar; a -> asa; t -> tir; s-> shamshir
    bool isUsed;
    Point cord;
    int count;
    int damage;
} Gun;

typedef struct
{
    char type;
    bool isUsed;
    Point cord;
    int count;
} Enchant;

typedef struct
{
    int count;
    char type;
    bool isUsed;
    Point cord;
} Gold;

typedef struct
{
    int damage;
    Point cord;
    bool isVisible;
} Trap;

typedef struct
{
    int from; // source level
    int to;   // dest level
    Point cord;
} Stair;

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
    Food *foods;
    Pillar pillar;
    Gold *golds;
    Gun *guns;
    Enchant *enchants;
    Trap *traps;
    Stair stair;
    Enemy *enemy;
    int keyCount;
    int foodCount;
    int goldCount;
    int gunCount;
    int enchantCount;
    int trapCount;
    int enemyCount;
    char type;
    int stairCount;
} Room;

typedef struct
{
    Point cord;
    int health;
    int state; // 0 -> in passway ; 1 -> in room
    Room *room;
    Passway *passway;
    Gun **guns;
    Enchant **enchants;
    int foodCount;
    Food *foods;
    int gunCount;
    int enchantCount;
    int level;
    char *name;
    int acientKey;
    int gold;
    int brokenAcientKey;
    int enemyCount;
    Enemy **enemies;
    int power;
} Player;

typedef struct
{
    Room **rooms;
    Passway **passways;
    Point key;
    int roomsCount;
    int level;
} Level;

typedef struct
{
    Level **levels;
    Player *player;

    int currentLevel;
} Game;

void startGame(User *user, Mix_Music *music);
void resumeGame(User *user, Mix_Music *music);
#endif