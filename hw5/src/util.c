#include "student/util.h"

int* str_to_int(char* str, int* num)
{
    int result = 0;
    while (str[0] != '\0' && str[0] != '\n')
    {
        if (str[0] < '0' || str[0] > '9')
        {
            return NULL;
        }
        result *= 10;
        result += str[0] - '0';
        str++;
    }
    *num = result;
    return num;
}

char* trim_string(char* str)
{
    char* w = str;
    while(*w != ' ' && *w != '\0' && *w != '\n')
        w++;
    if (*w == ' ')
        *w = '\0';
    return str;
}