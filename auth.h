#ifndef HEADER_H_AUTH
#define HEADER_H_AUTH
#include <stdbool.h>

typedef struct {
    int color;
    int level;
    char* music;
} Setting;

typedef struct
{
    unsigned long long int password;
    char *username;
    char *email;
    int games;
    int golds;
    int score;
    long long gameplay;
    bool isAuth;
    Setting setting;
} User;

char isAuthorized(User **);
int getUserId();
User *login(char *, char *, char ** , int);
User *registerUser(char *username, char *password, char *email, char **message);
void logout();
#endif