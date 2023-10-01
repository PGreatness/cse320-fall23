#ifndef ERR_STRING_H
#define ERR_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int is_valid_template_string(char** tmpt);
void print_args(char* current, char* end, va_list* ap, FILE* out);

#endif