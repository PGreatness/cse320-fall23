#include "child_processes.h"

int run_child_process(char* command, char* args[], int num_args)
{
    // create a child processs
    pid_t pid = fork();

    // check if fork() failed
    if (pid < 0)
    {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }

    // check if we are in the child process
    if (pid == 0)
    {
        // close stdout and redirect to stderr
        dup2(STDERR_FILENO, STDOUT_FILENO);
        fprintf(stdout, "I am child with pid %d", getpid());
        // start tracing the child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        // execute the command with execvp()
        execvp(command, args);

        // if we get here, execlp() failed
        perror("execvp() failed");
        exit(EXIT_FAILURE);
    }
    handle_signal_using_handler(SIGCHLD, handle_sigchild);
    // add the child to the list of children
    set_child_status(spawn_child(pid, 1, args, num_args), PSTATE_RUNNING);
    // we are in the parent process
    return pid;
}

void show_child_process(pid_t pid, FILE* stream)
{
    // get the child process
    child_t *child = get_child(pid);


    // check if the child exists
    if (child == NULL)
    {
        fprintf(stream, "Child %d not found\n", pid);
    }

    // print out the child's information
    char *status;
    switch (child->status)
    {
        case PSTATE_NONE:
            status = "none";
            break;
        case PSTATE_RUNNING:
            status = "running";
            break;
        case PSTATE_STOPPING:
            status = "stopping";
            break;
        case PSTATE_STOPPED:
            status = "stopped";
            break;
        case PSTATE_CONTINUING:
            status = "continuing";
            break;
        case PSTATE_KILLED:
            status = "killed";
            break;
        case PSTATE_DEAD:
            status = "dead";
            break;
        default:
            status = "unknown";
            break;
    }
    fprintf(stream, "%d\t%d\t%c\t%s\t", child->deetId, child->pid, child->trace ? 'T' : 'U', status);
    for (int i = 0; child->args[i] != NULL; i++)
    {
        fprintf(stream, "%s ", child->args[i]);
    }
    fprintf(stream, "\n");
}