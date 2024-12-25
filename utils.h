#include <string.h>

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
    for (int i = strlen(s)-1; i >=0; i--)
    {
        num += power * (s[i] - '0');
        power *= 10;
    }
    return num;
}