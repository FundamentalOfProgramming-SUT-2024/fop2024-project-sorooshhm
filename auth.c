#include <stdbool.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "db.h"

extern PGconn *conn;

char isAuthorized(User **user)
{
    FILE *fptr = fopen(".env", "r");
    if (fptr == NULL)
    {
        printf("Unable to load file");
        exit(1);
    }
    char isAuth;
    isAuth = fgetc(fptr);
    if (isAuth == '1')
    {
        fgetc(fptr);
        char username[100];
        fgets(username, 100, fptr);
        username[strlen(username) - 1] = '\0';
        // *user = findUser(username);
        (*user) = get_user_by_username(conn, username);
        printf("%d" , (*user)->games);
        
        (*user)->isAuth = true;
        (*user)->setting.color = 0;
        (*user)->setting.level = 1;
        (*user)->setting.music = "./music/1.mp3";
    }
    fclose(fptr);
    return isAuth;
};

int getUserId() {};

User *login(char *username, char *password, char **message, int forget)
{
    // User *user = findUser(username);
    User *user = get_user_by_username(conn, username);
    if (user == NULL)
    {
        *message = "Username is wrong";
        return NULL;
    }
    if (!forget && hash(password) != user->password)
    {
        *message = "Password is wrong";
        return NULL;
    }
    FILE *envFile = fopen(".env", "w");
    char content[100] = "1\n";
    strcat(content, username);
    fprintf(envFile, "%s\n", content);
    fclose(envFile);
    user->isAuth = true;
    (user)->setting.color = 0;
    (user)->setting.level = 1;
    (user)->setting.music = "./music/1.mp3";
    return user;
};

User *registerUser(char *username, char *password, char *email, char **message)
{
    // int check = userExixst(username);
    int check = check_username_exists(conn, username);
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
    (user)->setting.color = 0;
    (user)->setting.level = 1;
    (user)->setting.music = "./music/1.mp3";
    // insertUser(user);
    insert_user(conn, *user);
    FILE *envFile = fopen(".env", "w");
    char content[100] = "1\n";
    strcat(content, username);
    fprintf(envFile, "%s\n", content);
    fclose(envFile);
    user->isAuth = true;
    return user;
};

void logout()
{
    FILE *envFile = fopen(".env", "w");
    fprintf(envFile, "0");
    fclose(envFile);
    return;
}