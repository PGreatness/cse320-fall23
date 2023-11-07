#include "signaling.h"
#include "track_children.h"

void handle_sigchild(int sig)
{
    int status;
    pid_t child_pid = waitpid(-1, &status, WNOHANG);
    log_signal(sig);
    // fprintf(stderr, "child_pid = %d\n", child_pid);
    debug("Signal recieved, is we done? %s", WIFEXITED(status) ? "yes" : "no");
    child_t *child = get_child(child_pid);
    if (child == NULL)
    {
        debug("child not found");
        return;
    }
    // printf("child status: %s\n", WIFSTOPPED(status) ? "stopped" : "running");
    if (WIFSTOPPED(status))
    {
        // child is stopped, continue it
        // kill(child_pid, SIGSTOP);
        // fprintf(stderr, "changing child status to stopped in handle_sigchild, old = %s\n", child->status == PSTATE_NONE ? "none" : "running");
        set_child_status(child, PSTATE_STOPPED);
        child_summary(child, stdout);
        return;
    }
    if (WIFCONTINUED(status))
    {
        // child is continued, set it to running
        set_child_status(child, PSTATE_RUNNING);
        child_summary(child, stdout);
        return;
    }
    if (WIFEXITED(status))
    {
        // child is exited, set it to dead
        // check the child's exit status
        int exit_status = WEXITSTATUS(status);
        set_exit_status(child, exit_status);
        set_child_status(child, PSTATE_DEAD);
        child_summary(child, stdout);
        return;
    }
    if (WTERMSIG(status) == SIGKILL)
    {
        // child is killed, set it to killed
        set_exit_status(child, SIGKILL);
        set_child_status(child, PSTATE_DEAD);
        child_summary(child, stdout);
        return;
    }
}

void block_signal(int sig)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblock_signal(int sig)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void handle_signal_using_handler(int sig, void (*handler)(int))
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, sig);
    sa.sa_flags = SA_RESTART;
    if (sigaction(sig, &sa, NULL) == -1)
    {
        perror("sigaction() failed");
        exit(EXIT_FAILURE);
    }
}