#include <stdbool.h>

typedef struct
{
    unsigned long long int password;
    char *username;
    char *email;
    int games;
    int golds;
    int score;
    int gameplay;
} User;

char isAuthorized();
int getUserId();
void login();
void registerUser();
