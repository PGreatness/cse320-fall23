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
#include "signaling.h"
#include "commands.h"
#include "util.h"

#define MAX_LINE 256
#define MAX_ARG_SIZE 64

typedef struct state {
    int state;
    char* name;
} state_t;

extern COMMANDS commands[];
extern int suppress;
extern state_t all_states[];

#define NUM_STATES 7

// Function prototypes
/**
 * Asks for input from the user.
 * @param stream The stream to read from.
 * @param args The array of arguments to be filled.
 * @param num_args The number of arguments found.
 * @return The command number corresponding to the input.
 * These numbers are defined in `commands.h`. If the input is
 * valid but not a command, if the input is too large or too small,
 * or if `getline` returned an error, return `-2`. If the input is valid
 * and the first character is either a newline or a null terminator (signifying EOF),
 * return `-1` and set `num_args` to `0` and `-1` respectively.
 * Empty inputs return `-1`. The function will allocate
 * memory for the arguments, so the caller must free the memory.
 * The number of arguments is stored in `num_args`. If the number
 * of arguments is 0, `args` will not be allocated. Note that this
 * function can be called in a loop to get multiple inputs by
 * checking for return greater than -3.
*/
int get_input(FILE* stream, char* args[], int* num_args);

/**
 * Prints the help message.
*/
void print_help();

#endif //HW4_DEET_FUNC_H