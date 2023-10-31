#include <stdlib.h>

#include "deet.h"
#include "deet_func.h"
#include "free_used.h"
#include "signaling.h"

int main(int argc, char *argv[]) {
    // TO BE IMPLEMENTED
    // Remember: Do not put any functions other than main() in this file.
    handle_signal(SIGCHLD, sigchild_handler);
    // block_signal(SIGINT);
    char c;
    char *args[MAX_ARG_SIZE];
    int num_args;
    while ((c = ask_for_input(stdin, args, &num_args)) != -1)
    {
        switch (c)
        {
            case CMD_HELP:
                print_help();
                break;
            case CMD_QUIT:
                exit(EXIT_SUCCESS);
            case CMD_SHOW:
                break;
            case CMD_RUN:
                break;
            case CMD_STOP:
                // look at the args array
                // args[0] is the process id
                printf("Stopping process %s\n", args[0]);
                printf("num_args: %d\n", num_args);
                // free the args array
                free_args(args, num_args);
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
