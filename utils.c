#include <string.h>
#include <stdlib.h>
#include <time.h>

unsigned long long int hash(char *s)
{
    int p = 31;
    long long int m = 1e5 + 1;
    int power = 1;
    unsigned long long int res = 0;
    for (int i = 0; i < strlen(s); i++)
    {
        res += ((s[i]) * power) % m;
        power = (power * p) % m;
    }
    return res;
}

unsigned long long int stringToNumber(char *s)
{
    unsigned long long int num = 0;
    long long int power = 1;
    for (int i = strlen(s) - 1; i >= 0; i--)
    {
        num += power * (s[i] - '0');
        power *= 10;
    }
    return num;
}

char *generateRadomPass(char *pass)
{
    srand(time(NULL));

    char characters[52] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopkrstuvwxyz";
    for (int i = 0; i < 7; i++)
    {
        if (i == 2)
        {
            char randomletter = 'A' + (rand() % 26);
            pass[i] = randomletter;
        }
        else if (i == 4)
        {
            char randomletter = 'a' + (rand() % 26);
            pass[i] = randomletter;
        }
        else if (i == 6)
        {
            char randomletter = '0' + (rand() % 10);
            pass[i] = randomletter;
        }
        else
        {

            char randomletter = characters[(random() % 52)];
            pass[i] = randomletter;
        }
    }
    pass[7] = '\0';
    return pass;
}

int isChar(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}
int isCapital(char c)
{
    return ((c >= 'A' && c <= 'Z'));
}
int isSmall(char c)
{
    return ((c >= 'a' && c <= 'z'));
}

int isDigit(char c)
{
    return (c >= '0' && c <= '9');
}

int validateEmail(char *email)
{
    int len = strlen(email);
    if (!isChar(email[0]))
    {
        return 0;
    }
    int At = -1, Dot = -1;

    for (int i = 0; i < len; i++)
    {
        if (email[i] == '@')
        {

            At = i;
        }

        else if (email[i] == '.')
        {

            Dot = i;
        }
    }

    if (At == -1 || Dot == -1)
        return 0;

    if (At > Dot)
        return 0;

    return !(Dot >= (len - 1));
}

int validatePassword(char *pass)
{
    int len = strlen(pass);
    if (len < 7)
    {
        return 0;
    }
    int num, captial, small = 0;
    for (int i = 0; i < len; i++)
    {
        if (isDigit(pass[i]))
        {
            num = 1;
        }
        if (isCapital(pass[i]))
        {
            captial = 1;
        }
        if (isSmall(pass[i]))
        {
            small = 1;
        }
    }
    return (num != 0) && (small != 0) && (captial != 0);
}