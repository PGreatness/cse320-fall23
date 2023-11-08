#include "signaling.h"
#include "track_children.h"

int shutdown = 0;

void handle_sigchild(int sig)
{
    if (shutdown)
    {
        child_t* s = sentinel.next;
        int has_children = 0;
        while (s != &sentinel)
        {
            if (s->status == PSTATE_DEAD)
            {
                s = s->next;
                continue;
            }
            has_children = 1;
            log_signal(sig);
            set_child_status(s, PSTATE_KILLED, 0);
            kill(s->pid, SIGKILL);
            int status;
            waitpid(s->pid, &status, 0);
            log_signal(sig);
            set_child_status(s, PSTATE_DEAD, status);
            free_child(s);
            s = s->next;
        }

        if (!has_children)
            log_signal(sig);
        log_shutdown();
        exit(EXIT_SUCCESS);
    }
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        log_signal(sig);
    debug("Signal recieved, is we done? %s, child: %i", WIFEXITED(status) ? "yes" : "no", child_pid);
    // if (errno)
    //     perror("waitpid() failed");
    child_t *child = get_child(child_pid);
    if (child == NULL)
    {
        debug("child not found");
        return;
    }
    if (WTERMSIG(status) == SIGKILL)
    {
        // child is killed, set it to killed
        block_signal(SIGCHLD);
        set_exit_status(child, SIGKILL);
        set_child_status(child, PSTATE_DEAD, status);
        child_summary(child, STDOUT_FILENO);
        free_child(child);
        child->deetId = -1;
        unblock_signal(SIGCHLD);
        return;
    }
    if (WIFEXITED(status))
    {
        // child is exited, set it to dead
        // check the child's exit status
        block_signal(SIGCHLD);
        int exit_status = WEXITSTATUS(status);
        set_exit_status(child, exit_status);
        set_child_status(child, PSTATE_DEAD, exit_status);
        child_summary(child, STDOUT_FILENO);
        // free_child(child);
        decrement_next_deet_id();
        unblock_signal(SIGCHLD);
        return;
    }
    if (WIFSTOPPED(status))
    {
        block_signal(SIGCHLD);
        set_child_status(child, PSTATE_STOPPED, 0);
        child_summary(child, STDOUT_FILENO);
        unblock_signal(SIGCHLD);
        return;
    }
    if (WIFCONTINUED(status))
    {
        // child is continued, set it to running
        block_signal(SIGCHLD);
        set_child_status(child, PSTATE_RUNNING, 0);
        child_summary(child, STDOUT_FILENO);
        unblock_signal(SIGCHLD);
        return;
    }
    }
}

void handle_sigint(int sig)
{
    shutdown = 1;
    log_signal(sig);
    unblock_signal(SIGCHLD);
    kill(getpid(), SIGCHLD);
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