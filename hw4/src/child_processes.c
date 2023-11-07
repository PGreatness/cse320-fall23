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
        // fprintf(stdout, "I am child with pid %d", getpid());
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
    child_t *child = spawn_child(pid, 1, args, num_args);
    child_summary(child, stdout);
    // we are in the parent process
    return pid;
}

void show_child_process(pid_t deet_id, FILE* stream)
{
    // get the child process
    child_t *child = get_child_by_deet_id(deet_id);


    // check if the child exists
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deet_id);
        return;
    }

    child_summary(child, stream);
}

int continue_child_process(int deet_id)
{
    // get the child process
    child_t *child = get_child_by_deet_id(deet_id);

    // check if the child exists
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deet_id);
        return -1;
    }

    // check if the child is stopped
    if (child->status != PSTATE_STOPPED)
    {
        debug("Child with deet_id %d is not stopped\n", deet_id);
        return -1;
    }

    // continue the child process
    ptrace(PTRACE_CONT, child->pid, NULL, NULL);
    set_child_status(child, PSTATE_RUNNING);
    child_summary(child, stdout);
    return 0;
}

int kill_child_process(int deet_id)
{
    // get the child process
    child_t *child = get_child_by_deet_id(deet_id);

    // check if the child exists
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deet_id);
        return -1;
    }

    // check if the child is stopped
    if (child->status != PSTATE_STOPPED)
    {
        debug("Child with deet_id %d is not stopped\n", deet_id);
        return -1;
    }

    // kill the child process
    kill_child(child->pid);
    return 0;
}