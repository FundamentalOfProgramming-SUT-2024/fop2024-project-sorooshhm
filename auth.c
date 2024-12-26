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
    char content[100] = "1\n";
    strcat(content, username);
    // printf("%s\n", content);
    fprintf(envFile, content);
    fclose(envFile);
    return user;
};

User *registerUser(char *username, char *password, char *email, char **message)
{
    int check = userExixst(username);
    if (check)
    {
        *message = "This username is taken";
        return NULL;
    }
    User *user = (User *)malloc(sizeof(User));
    user->username = username;
    user->email = email;
    user->password = hash(password);
    user->games = 0;
    user->golds = 0;
    user->score = 0;
    user->gameplay = 0;
    insertUser(user);
    FILE *envFile = fopen(".env", "w");
    char content[100] = "1\n";
    strcat(content, username);
    // printf("%s\n", content);
    fprintf(envFile, content);
    fclose(envFile);
    return user;
};
