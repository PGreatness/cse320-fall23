#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

void fatal(char *fmt, ...);
void error(char *fmt, ...);
void warning(char *fmt, ...);
void debug(char *fmt, ...);

#endif