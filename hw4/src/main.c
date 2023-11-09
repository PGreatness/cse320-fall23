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
    block_signal(SIGCHLD, NULL);
    while ((c = get_input(stdin, args, &num_args)) != -1)
    {
        block_signal(SIGCHLD, NULL);
        switch (c)
        {
            case CMD_HELP:
                print_help();
                break;
            case CMD_QUIT:
                if (num_args != 0)
                {
                    debug("Error: quit command does not take arguments\n");
                    log_error(commands[CMD_QUIT].name);
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
                    log_error(commands[CMD_SHOW].name);
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
                    log_error(commands[CMD_RUN].name);
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
                    log_error(commands[CMD_STOP].name);
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
                    log_error(commands[CMD_CONT].name);
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    break;
                }
                continue_child_process(deet_id, STDOUT_FILENO);
                break;
            case CMD_RELEASE:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_RELEASE].name);
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    break;
                }
                release_child_process(deet_id);
                break;
            case CMD_WAIT:
                if (num_args < 1)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_WAIT].name);
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    break;
                }
                int state = PSTATE_DEAD;
                if (num_args > 1 && (state = contains_state(args[1])) == -1)
                {
                    debug("Error: invalid command %s\n", args[1]);
                    log_error(commands[CMD_WAIT].name);
                    break;
                }
                wait_for_child_process(deet_id, state);
                break;
            case CMD_KILL:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_KILL].name);
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
