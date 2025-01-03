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
    char type; // 'n' -> normal 's' -> secret 'l' -> locked
    int password;
} Door;

typedef struct
{
    int health;
    bool isUsed;
    Point cord;
} Food;

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
    Trap *traps;
    Stair stair;
    int keyCount;
    int foodCount;
    int trapCount;
    int stairCount;
} Room;

typedef struct
{
    Point cord;
    int health;
    int state; // 0 -> in passway ; 1 -> in room
    Room *room;
    Passway *passway;
    int foodCount;
    Food **foods;
    int level;
    char *name;
    int *usedFood;
    int acientKey;
    int brokenAcientKey;
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