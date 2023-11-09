#ifndef HW4_UTIL_H
#define HW4_UTIL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "deet.h"
#include "deet_func.h"

int* str_to_int(char *str, int *num);

int print_string(int fileno, char *str);

int print_int(int fileno, int var);

int contains_state(char* str);

char* trim_string(char *str);

#endif //HW4_UTIL_H