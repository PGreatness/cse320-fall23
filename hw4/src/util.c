#include "util.h"

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

int print_string(int fileno, char* str)
{
    ssize_t bytes_written;
    size_t len = strlen(str);
    while (len > 0)
    {
        do
        {
            bytes_written = write(fileno, str, len);
        } while ((bytes_written < 0) && (errno == EINTR));
        if (bytes_written < 0)
            return -1;
        len -= bytes_written;
        str += bytes_written;
    }
    return 0;
}

int int_size(int p)
{
    int i = 1;
    while (p /= 10)
        i++;
    return i;
}

char* int_to_str(int p)
{
    int a;
    char* s = calloc((a = int_size(p)), sizeof(char));
    char* w = s + a - 1;
    while (w >= s)
    {
        *w = (p % 10) + '0';
        p /= 10;
        w -= sizeof(char);
    }
    return s;
}

int print_int(int fileno, int var)
{
    char* i = int_to_str(var);
    if (print_string(fileno, i) == -1)
    {
        free(i);
        return -1;
    }
    free(i);
    return 0;
}
