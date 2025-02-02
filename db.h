#ifndef HEADER_H_DB
#define HEADER_H_DB

#include <postgresql/libpq-fe.h>
#include "auth.h"
#include "game.h"

User *findUser(char *username);
User **find(int *);
int userExixst(char *username);
void insertUser(User *user);
void updateUser(User *user);
void saveGame(Game *game, User *user);
int loadGame(Game *game, User *user);
void finish_with_error(PGconn *conn);
PGconn *connect_db();
void insert_user(PGconn *conn, User user);
void update_user(PGconn *conn, User user);
User **get_users(PGconn *conn , int* count);
User *get_user_by_username(PGconn *conn, const char *username);
int check_username_exists(PGconn *conn, const char *username);

#endif