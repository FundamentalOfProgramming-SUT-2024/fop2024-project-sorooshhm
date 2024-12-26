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
User *login(char *, char *, char **);
User *registerUser(char *username, char *password, char *email, char **message);