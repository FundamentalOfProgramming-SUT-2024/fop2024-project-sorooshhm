#include <stdbool.h>
#include <stdio.h>
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

void login() {};

void registerUser() {};
