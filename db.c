#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "auth.h"
#include "utils.h"

User *findUser(char *username)
{
    FILE *db = fopen("./users.dev.db", "r");
    char data[10000];
    char c;
    int index = 0;
    while (c != '#')
    {
        c = fgetc(db);
        data[index++] = c;
    }
    data[index--] = '\0';
    char name[100];
    int j = 0;
    for (int i = 0; i < index; i++)
    {
        if (data[i] == '\n')
        {
            name[j] = '\0';
            break;
        }
        name[j++] = data[i];
    }

    j = 0;
    if (strcmp(name, username) == 0)
    {
        User *user = (User *)malloc(1 * sizeof(User));
        user->username = (char *)malloc(100 * sizeof(char));
        user->email = (char *)malloc(100 * sizeof(char));
        char value[100];
        int key = 0;
        for (int i = 0; i < index; i++)
        {
            if (data[i] == '\n')
            {
                i++;
                value[j] = '\0';
                j = 0;
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
            value[j++] = data[i];
        }

        return user;
    }
    return NULL;
}

int main()
{
    User *soroosh = findUser("soroosh");
    printf("username %s email %s password %lld", soroosh->username, soroosh->email, soroosh->password);
    return 0;
}