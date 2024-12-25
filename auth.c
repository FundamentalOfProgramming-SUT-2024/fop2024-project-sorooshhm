#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void login(char *username, char *password)
{
    char *name = "$$";
   
};

void registerUser() {};
