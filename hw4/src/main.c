#include <stdlib.h>

#include "deet.h"
#include "deet_func.h"
#include "free_used.h"
#include "signaling.h"
#include "child_processes.h"
#include "track_children.h"
#include "debug.h"
#include "util.h"

int main(int argc, char *argv[]) {
    // TO BE IMPLEMENTED
    // Remember: Do not put any functions other than main() in this file.
    char c;
    c = getopt(argc, argv, "p");
    switch (c)
    {
        case 'p':
            suppress = 1;
            break;
        default:
            break;
    }
    char *args[MAX_ARG_SIZE] = {NULL};
    int num_args;
    int deet_id;
    log_startup();
    handle_signal_using_handler(SIGINT, handle_sigint);
    handle_signal_using_handler(SIGCHLD, handle_sigchild);
    // block the SIGCHLD signal
    block_signal(SIGCHLD);
    while ((c = get_input(stdin, args, &num_args)) != -1)
    {
        block_signal(SIGCHLD);
        switch (c)
        {
            case CMD_HELP:
                if (num_args != 0)
                {
                    debug("Error: help command does not take arguments\n");
                    log_error("Help command does not take arguments");
                    break;
                }
                print_help();
                break;
            case CMD_QUIT:
                if (num_args != 0)
                {
                    debug("Error: quit command does not take arguments\n");
                    log_error("Quit command does not take arguments");
                    break;
                }
                // free the args array
                free_args(args, num_args);
                // free the children
                free_children();
                break;
            case CMD_SHOW:
                // show the process specified
                if (num_args == 0)
                {
                    show_all_child_processes(STDOUT_FILENO);
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    break;
                }
                show_child_process(deet_id, STDOUT_FILENO);
                break;
            case CMD_RUN:
                // look at the args array
                // args[0] is the command
                // args[1,...] are the arguments
                // num_args includes the command
                if (num_args == 0)
                {
                    debug("Error: no command specified\n");
                    log_error("No command specified");
                    break;
                }
                debug("Running command %s\n", args[0]);
                run_child_process(args[0], args, num_args);
                // free the args array
                free_args(args, num_args);
                break;
            case CMD_STOP:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error("No input specified");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    break;
                }
                stop_child_process(deet_id);
                break;
            case CMD_CONT:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error("No process specified");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error("Invalid process id");
                    break;
                }
                continue_child_process(deet_id, STDOUT_FILENO);
                break;
            case CMD_RELEASE:
                break;
            case CMD_WAIT:
                break;
            case CMD_KILL:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error("No process specified");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error("Invalid process id");
                    break;
                }
                kill_child_process(deet_id);
                break;
            case CMD_PEEK:
                break;
            case CMD_POKE:
                break;
            case CMD_BT:
                break;

            default:
                // print out a ?
                log_error(args[0] != 0 ? args[0] : "");
                print_string(STDOUT_FILENO, "?\n");
                // free the args array
                free_args(args, num_args);
                if (num_args == -1)
                {
                    // an empty input was given, quit the program
                    // free the children
                    free_children();
                }
                break;
        }
    }

    // abort();
}
