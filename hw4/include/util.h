#ifndef HW4_UTIL_H
#define HW4_UTIL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "deet.h"
#include "deet_func.h"

int replace_tabs_with_space(char *str);

int* str_to_int(char *str, int *num);

long hex_str_to_long(char *hex_str);

int print_string(int fileno, char *str);

int print_int(int fileno, int var);

int print_int_as_hex(int fileno, int var);

int print_long_as_hex(int fileno, unsigned long var);

int contains_state(char* str);

char* trim_string(char *str);

#endif //HW4_UTIL_H