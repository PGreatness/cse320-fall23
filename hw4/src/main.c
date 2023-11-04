#include <stdlib.h>

#include "deet.h"
#include "deet_func.h"
#include "free_used.h"
#include "signaling.h"
#include "child_processes.h"
#include "track_children.h"
#include "debug.h"

int main(int argc, char *argv[]) {
    // TO BE IMPLEMENTED
    // Remember: Do not put any functions other than main() in this file.
    char c;
    char *args[MAX_ARG_SIZE] = {NULL};
    int num_args;
    while ((c = get_input(stdin, args, &num_args)) != -1)
    {
        switch (c)
        {
            case CMD_HELP:
                print_help();
                break;
            case CMD_QUIT:
                // free the args array
                free_args(args, num_args);
                // free the children
                free_children();
                exit(EXIT_SUCCESS);
            case CMD_SHOW:
                // show the process specified
                // UNSAFE: atoi() is not safe, replace later
                show_child_process(atoi(args[0]), stdout);
                break;
            case CMD_RUN:
                // look at the args array
                // args[0] is the command
                // args[1,...] are the arguments
                // num_args includes the command
                debug("Running command %s\n", args[0]);
                run_child_process(args[0], args, num_args);
                // free the args array
                free_args(args, num_args);
                break;
            case CMD_STOP:
                // testing: kill the last child
                kill_child(get_last_child()->pid);
                break;
            case CMD_CONT:
                break;
            case CMD_RELEASE:
                break;
            case CMD_WAIT:
                break;
            case CMD_KILL:
                break;
            case CMD_PEEK:
                break;
            case CMD_POKE:
                break;
            case CMD_BT:
                break;

            default:
                // print out a ?
                fprintf(stdout, "?\n");
                break;
        }
    }

    // abort();
}
