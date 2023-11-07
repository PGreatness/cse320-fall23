#include "signaling.h"
#include "track_children.h"

int shutdown = 0;

void handle_sigchild(int sig)
{
    // block the signal
    block_signal(SIGCHLD);
    if (shutdown)
    {
        // unblock_signal(SIGCHLD);
        child_t* s = sentinel.next;
        while (s != &sentinel)
        {
            if (s->status == PSTATE_DEAD)
            {
                s = s->next;
                continue;
            }
            set_child_status(s, PSTATE_KILLED);
            kill(s->pid, SIGKILL);
            log_signal(sig);
            set_child_status(s, PSTATE_DEAD);
            free_child(s);
            s = s->next;
        }

        log_shutdown();
        exit(EXIT_SUCCESS);
    }
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        log_signal(sig);
    debug("Signal recieved, is we done? %s, child: %i", WIFEXITED(status) ? "yes" : "no", child_pid);
    if (errno)
        perror("waitpid() failed");
    child_t *child = get_child(child_pid);
    if (child == NULL)
    {
        debug("child not found");
        unblock_signal(SIGCHLD);
        return;
    }
    if (WTERMSIG(status) == SIGKILL)
    {
        // child is killed, set it to killed
        set_exit_status(child, SIGKILL);
        set_child_status(child, PSTATE_DEAD);
        child_summary(child, stdout);
        free_child(child);
        child->deetId = -1;
        unblock_signal(SIGCHLD);
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
        free_child(child);
        unblock_signal(SIGCHLD);
        return;
    }
    if (WIFSTOPPED(status))
    {
        set_child_status(child, PSTATE_STOPPED);
        child_summary(child, stdout);
        unblock_signal(SIGCHLD);
        return;
    }
    if (WIFCONTINUED(status))
    {
        // child is continued, set it to running
        set_child_status(child, PSTATE_RUNNING);
        child_summary(child, stdout);
        unblock_signal(SIGCHLD);
        return;
    }
    }
}

void handle_sigint(int sig)
{
    pid_t child_pid;
    child_t* s = sentinel.next;
    while (s != &sentinel)
    {
        if (s->status != PSTATE_DEAD)
            kill_child(s->pid);
        s = s->next;
    }
    while ((child_pid = waitpid(-1, NULL, 0)) > 0)
    {
        log_signal(sig);
        child_t *child = get_child(child_pid);
        if (child == NULL)
        {
            debug("child not found");
            // return;
            continue;
        }
        set_exit_status(child, SIGINT);
        set_child_status(child, PSTATE_DEAD);
        child_summary(child, stdout);
        free_child(child);
    }
    log_shutdown();
    exit(EXIT_SUCCESS);
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