#include <stdbool.h>

typedef struct
{
    char *name;
    char *username;
    char *email;
    int games;
    int golds;
    int score;
    int gameplay;
} User;

bool isAuthorized();
int getUserId();
void login();
void registerUser();
