#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

void fatal(char *fmt, ...);
void error(char *fmt, char *a1, char *a2, char *a3, char *a4, char *a5, char *a6);
void warning(char *fmt, char *a1, char *a2, char *a3, char *a4, char *a5, char *a6);
void debug(char *fmt, char *a1, char *a2, char *a3, char *a4, char *a5, char *a6);

#endif