#include "util.h"
#include "signaling.h"

int replace_tabs_with_space(char* str)
{
    int num_tabs = 0;
    char* w = str;
    while (*w != '\0' && *w != '\n')
    {
        if (*w == '\t')
        {
            *w = ' ';
            num_tabs++;
        }
        w++;
    }
    return num_tabs;
}

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

long hex_str_to_long(char* hex_str)
{
    char* end;
    if (hex_str[0] == '\0' || hex_str[0] == '\n')
        return -1;
    if (strlen(hex_str) < 2)
        return 0;
    if (hex_str[0] == '0' && hex_str[1] == 'x')
        hex_str += 2;
    return strtoul(hex_str, &end, 16);
}

int print_string(int fileno, char* str)
{
    printf("%s", str);
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

int print_int_as_hex(int fileno, int var)
{
    char* hex_str = calloc(9, sizeof(char));
    sprintf(hex_str, "%x", var);
    if (print_string(fileno, hex_str) == -1)
    {
        free(hex_str);
        return -1;
    }
    free(hex_str);
    return 0;
}

int print_long_as_hex(int fileno, unsigned long long_num)
{
    char* hex_str = calloc(17, sizeof(char));
    sprintf(hex_str, "%016lx", long_num);
    if (print_string(fileno, hex_str) == -1)
    {
        free(hex_str);
        return -1;
    }
    free(hex_str);
    return 0;
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

int contains_state(char* str)
{
    for (int i = 0; i < NUM_STATES; i++)
    {
        if (strcmp(str, all_states[i].name) == 0)
            return all_states[i].state;
    }
    return -1;
}