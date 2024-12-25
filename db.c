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
    char value[100];
    int key = 0;
    while (fgets(data, 100, db))
    {
        if (key == 0)
        {
            strcpy(user->username, value);
        }
        else if (key == 1)
        {
            user->password = stringToNumber(value);
        }
        else if (key == 2)
        {
            strcpy(user->email, value);
        }
        else if (key == 3)
        {
            user->games = stringToNumber(value);
        }
        else if (key == 4)
        {
            user->golds = stringToNumber(value);
        }
        else if (key == 5)
        {
            user->score = stringToNumber(value);
        }
        else if (key == 6)
        {
            user->gameplay = stringToNumber(value);
        }
        key++;
    }

    return user;
}

// int main()
// {
//     User *soroosh = findUser("soroosh");
//     printf("%d\n", soroosh == NULL);
//     // printf("username %s email %s password %lld", soroosh->username, soroosh->email, soroosh->password);
//     return 0;
// }