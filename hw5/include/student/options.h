#ifndef HW5_STUDENT_OPTIONS_H
#define HW5_STUDENT_OPTIONS_H

#define STUDENT_OPTION_COUNT 3

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

typedef struct {
    // The short name of the option, e.g. 'h' for -h
    const char short_name;
    // The long name of the option, e.g. "help" for --help
    const char *long_name;
    // A description of the option
    const char *description;
    // Whether the option is required. 1 for required, 0 for optional
    const int is_required;
    // Whether the option takes an argument. 1 for takes an argument, 0 for no argument
    const int has_arg;
} command_line_args_t;

extern const command_line_args_t STUDENT_OPTIONS[];

extern const char* STUDENT_SHORT_OPTIONS;

extern const struct option STUDENT_LONG_OPTIONS[];

#endif // HW5_STUDENT_OPTIONS_H