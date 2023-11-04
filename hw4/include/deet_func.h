#ifndef HW4_DEET_FUNC_H
#define HW4_DEET_FUNC_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "deet.h"
#include "debug.h"
#include "commands.h"

#define MAX_LINE 256
#define MAX_ARG_SIZE 64

typedef struct cmds
{
    int command;
    char* name;
    char* desc;
    int min_args;
    int max_args;
} COMMANDS;

extern COMMANDS commands[];

// Function prototypes
/**
 * Asks for input from the user.
 * @param stream The stream to read from.
 * @param args The array of arguments to be filled.
 * @param num_args The number of arguments found.
 * @return The command number corresponding to the input.
 * These numbers are defined in `commands.h`. If the input is
 * valid but not a command, return `-2`. If the input is invalid,
 * return `-1`. Empty inputs return `-2`. The function will allocate
 * memory for the arguments, so the caller must free the memory.
 * The number of arguments is stored in `num_args`. If the number
 * of arguments is 0, `args` will not be allocated.
*/
int get_input(FILE* stream, char* args[], int* num_args);

/**
 * Prints the help message.
*/
void print_help();

#endif //HW4_DEET_FUNC_H