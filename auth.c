#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "db.h"

char isAuthorized()
{
    FILE *fptr = fopen(".env", "r");
    if (fptr == NULL)
    {
        printf("Unable to load file");
        exit(1);
    }
    char isAuth;
    isAuth = fgetc(fptr);
    fclose(fptr);
    return isAuth;
};

int getUserId() {};

User *login(char *username, char *password, char **message)
{
    User *user = findUser(username);
    if (user == NULL)
    {
        *message = "Username is wrong";
        return NULL;
    }
    if (hash(password) != user->password)
    {
        *message = "Password is wrong";
        return NULL;
    }
    FILE *envFile = fopen(".env", "w");
    char *content = "1\n";
    strcat(content, username);
    fputs(content, envFile);
    fclose(envFile);
    return user;
};

void registerUser() {};
