#ifndef HEADER_H_DB
#define HEADER_H_DB

#include "auth.h"
#include "game.h"

User *findUser(char *username);
User **find(int *);
int userExixst(char *username);
void insertUser(User *user);
void updateUser(User *user);
void saveGame(Game *game, User *user);
void loadGame(Game *game, User *user);
#endif