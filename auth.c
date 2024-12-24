#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

bool isAuthorized()
{
    FILE *fptr = fopen(".env", "r");
    if (fptr == NULL)
    {
        printf("Unable to load file");
        exit(1);
    }
    char isAuth;
    isAuth =  fgetc(fptr);
    fclose(fptr);
    return isAuth == '1';
};

int getUserId() {};

void login() {};

void registerUser() {};
