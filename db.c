#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "auth.h"
#include "utils.h"

User *findUser(char *username)
{
    char *name = (char *)malloc(100 * sizeof(char));
    FILE *usernames = fopen("./users/usernames.db", "r");
    int check = 0;
    while (fgets(name, 100, usernames))
    {
        if (strcmp(name, username) == 0)
        {
            check = 1;
            break;
        }
    }
    fclose(usernames);
    if (check == 0)
    {
        return NULL;
    }
    char address[100] = "./users/";

    strcat(address, name);
    strcat(address, ".db");
    FILE *db = fopen(address, "r");
    char data[100];
    char c;
    int index = 0;
    User *user = (User *)malloc(1 * sizeof(User));
    user->username = (char *)malloc(100 * sizeof(char));
    user->email = (char *)malloc(100 * sizeof(char));
    int key = 0;
    while (fgets(data, 100, db))
    {
        data[strlen(data) - 1] = '\0';
        if (key == 0)
        {
            strcpy(user->username, data);
        }
        else if (key == 1)
        {
            user->password = stringToNumber(data);
        }
        else if (key == 2)
        {
            strcpy(user->email, data);
        }
        else if (key == 3)
        {
            user->games = stringToNumber(data);
        }
        else if (key == 4)
        {
            user->golds = stringToNumber(data);
        }
        else if (key == 5)
        {
            user->score = stringToNumber(data);
        }
        else if (key == 6)
        {
            user->gameplay = stringToNumber(data);
        }
        key++;
    }

    return user;
}

void insertUser(User *user)
{
    FILE *usernames = fopen("./users/usernames.db", "a");
    fprintf(usernames,"\n%s" ,user->username);
    fclose(usernames);
    char address[100] = "./users/";

    strcat(address, user->username);
    strcat(address, ".db");
    FILE *db = fopen(address, "w");
    fprintf(db,"%s\n", user->username);
    fprintf(db,"%lld\n", user->password);
    fprintf(db,"%s\n", user->email);
    fprintf(db,"%d\n", user->games);
    fprintf(db,"%d\n", user->golds);
    fprintf(db,"%d\n", user->score);
    fprintf(db,"%d\n", user->gameplay);
    fclose(db);
}

int userExixst(char *username)
{
    char *name = (char *)malloc(100 * sizeof(char));
    FILE *usernames = fopen("./users/usernames.db", "r");
    int check = 0;
    while (fgets(name, 100, usernames))
    {
        if (strcmp(name, username) == 0)
        {
            check = 1;
            break;
        }
    }
    fclose(usernames);
    if (check == 0)
    {
        return 0;
    }
    return 1;
}

// int main()
// {
//     User *soroosh = findUser("soroosh");
//     printf("%d\n", soroosh == NULL);
//     // printf("username %s email %s password %lld", soroosh->username, soroosh->email, soroosh->password);
//     return 0;
// }