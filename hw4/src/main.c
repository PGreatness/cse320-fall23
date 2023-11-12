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
    int c;
    c = getopt(argc, argv, "p");
    switch (c)
    {
        case 'p':
            suppress = 1;
            break;
        default:
            break;
    }
    c = -1;
    char *args[MAX_ARG_SIZE] = {NULL};
    int num_args;
    int deet_id;
    unsigned long addr;
    int limit;
    int state;
    log_startup();
    handle_signal_using_handler(SIGINT, handle_sigint);
    handle_signal_using_handler(SIGCHLD, handle_sigchild);
    // block the SIGCHLD signal
    block_signal(SIGCHLD, NULL);
    while ((c = get_input(stdin, args, &num_args)) > -3)
    {
        state = PSTATE_DEAD;
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
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                // free the args array
                free_args(args, num_args);
                // free the children
                eof_flag = 1;
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
                    print_string(STDOUT_FILENO, "?\n");
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
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                debug("Running command %s\n", args[0]);
                if (run_child_process(args[0], args, num_args) < 0)
                {
                    debug("Error with running command %s\n", args[0]);
                    log_error(commands[CMD_RUN].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                // free the args array
                free_args(args, num_args);
                break;
            case CMD_STOP:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_STOP].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error(commands[CMD_STOP].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (stop_child_process(deet_id) < 0)
                {
                    debug("Error: deet_id %d is not running\n", deet_id);
                    log_error(commands[CMD_STOP].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;
            case CMD_CONT:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_CONT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error(commands[CMD_CONT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (continue_child_process(deet_id, STDOUT_FILENO) < 0)
                {
                    debug("Error: deet_id %d is not stopped\n", deet_id);
                    log_error(commands[CMD_CONT].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;
            case CMD_RELEASE:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_RELEASE].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error(commands[CMD_RELEASE].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (release_child_process(deet_id) < 0)
                {
                    debug("Error: deet_id %d is not stopped\n", deet_id);
                    log_error(commands[CMD_RELEASE].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;
            case CMD_WAIT:
                if (num_args < 1)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_WAIT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error(commands[CMD_WAIT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                state = PSTATE_DEAD;
                if (num_args > 1 && (state = contains_state(args[1])) < 0)
                {
                    debug("Error: invalid command %s\n", args[1]);
                    log_error(commands[CMD_WAIT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                wait_for_child_process(deet_id, state);
                break;
            case CMD_KILL:
                if (num_args == 0)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_KILL].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error("Invalid process id");
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (kill_child_process(deet_id) < 0)
                {
                    debug("Error: deet_id %d is not running\n", deet_id);
                    log_error(commands[CMD_KILL].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;
            case CMD_PEEK:
                if (num_args < 2 || num_args > 3)
                {
                    debug("Error: no deet_id or address specified\n");
                    log_error(commands[CMD_PEEK].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                limit = 1;
                if (num_args == 3)
                {
                    if (str_to_int(args[2], &limit) == NULL)
                    {
                        debug("Error: invalid limit of %s\n", args[2]);
                        log_error(commands[CMD_PEEK].name);
                        print_string(STDOUT_FILENO, "?\n");
                        break;
                    }
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error(commands[CMD_PEEK].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                // peek at the address
                if ((addr = hex_str_to_long(args[1])) == 0)
                {
                    debug("Error: invalid address of %s\n", args[1]);
                    log_error(commands[CMD_PEEK].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (peek_in_process(deet_id, addr, limit, STDOUT_FILENO) < 0)
                {
                    debug("Error: peeking error\n");
                    log_error(commands[CMD_PEEK].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;
            case CMD_POKE:
                if (num_args != 3)
                {
                    debug("Error: no deet_id or address specified\n");
                    log_error(commands[CMD_POKE].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error(commands[CMD_POKE].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                // use limit as a temporary variable for the new value
                limit = 0;
                warn("args[2] = %s\n", args[2]);
                if (str_to_int(args[2], &limit) == NULL)
                {
                    debug("Error: invalid value of %s\n", args[2]);
                    log_error(commands[CMD_POKE].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                // peek at the address
                if ((addr = hex_str_to_long(args[1])) == 0)
                {
                    debug("Error: invalid address of %s\n", args[1]);
                    log_error(commands[CMD_POKE].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                unsigned long amnt = limit;
                warn("amnt = %lu\n", amnt);
                if (poke_in_process(deet_id, addr, amnt) < 0)
                {
                    debug("Error: poking error\n");
                    log_error(commands[CMD_POKE].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;
            case CMD_BT:
                if (num_args < 1 || num_args > 2)
                {
                    debug("Error: no deet_id specified\n");
                    log_error(commands[CMD_BT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (str_to_int(args[0], &deet_id) == NULL)
                {
                    debug("Error: invalid deet_id of %s\n", args[0]);
                    log_error(commands[CMD_BT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                limit = 10;
                if (num_args == 2 && (str_to_int(args[1], &limit) == NULL))
                {
                    debug("Error: invalid limit of %s\n", args[1]);
                    log_error(commands[CMD_BT].name);
                    print_string(STDOUT_FILENO, "?\n");
                    break;
                }
                if (backtrace_child_process(deet_id, limit, STDOUT_FILENO) < 0)
                {
                    debug("Error: deet_id %d is not running\n", deet_id);
                    log_error(commands[CMD_BT].name);
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;

            default:
                // print out a ?
                free_args(args, num_args);
                if (c == -1 && num_args == 0)
                {
                    // an empty input was given, quit the program
                    break;
                }
                // free the args array
                if (num_args == -1)
                {
                    // an empty input was given, quit the program
                    // free the children
                    eof_flag = 1;
                    free_children();
                } else {
                    log_error(args[0] != 0 ? args[0] : "");
                    print_string(STDOUT_FILENO, "?\n");
                }
                break;
        }
    }

    // abort();
}
