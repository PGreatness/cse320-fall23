#include "signaling.h"
#include "track_children.h"

int shutdown = 0;
int eof_flag = 0;

void handle_sigchild(int sig)
{
    if (shutdown)
    {
        child_t* s = sentinel.next;
        int has_children = 0;
        while (s != &sentinel)
        {
            has_children = 1;
            if (s->status == PSTATE_DEAD)
            {
                s = s->next;
                continue;
            }
            // log_signal(sig);
            set_child_status(s, PSTATE_KILLED, -1);
            child_summary(s, STDOUT_FILENO);
            kill(s->pid, SIGKILL);
            int status;
            waitpid(s->pid, &status, 0);
            log_signal(sig);
            set_child_status(s, PSTATE_DEAD, status);
            child_summary(s, STDOUT_FILENO);
            free_child(s);
            s = s->next;
        }

        if (!has_children && !eof_flag)
            log_signal(sig);
        log_shutdown();
        exit(EXIT_SUCCESS);
    }
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
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
        block_signal(SIGCHLD, NULL);
        set_exit_status(child, SIGKILL);
        set_child_status(child, PSTATE_DEAD, status);
        child_summary(child, STDOUT_FILENO);
        // free_child(child);
        // child->deetId = -1;
        unblock_signal(SIGCHLD, NULL);
        kill(getpid(), SIGCHLD);
        return;
    }
    if (WIFEXITED(status))
    {
        // child is exited, set it to dead
        // check the child's exit status
        block_signal(SIGCHLD, NULL);
        int exit_status = WEXITSTATUS(status);
        set_exit_status(child, exit_status);
        set_child_status(child, PSTATE_DEAD, exit_status);
        child_summary(child, STDOUT_FILENO);
        // free_child(child);
        decrement_next_deet_id();
        unblock_signal(SIGCHLD, NULL);
        kill(getpid(), SIGCHLD);
        return;
    }
    if (WIFSTOPPED(status))
    {
        block_signal(SIGCHLD, NULL);
        set_child_status(child, PSTATE_STOPPED, -1);
        child_summary(child, STDOUT_FILENO);
        unblock_signal(SIGCHLD, NULL);
        return;
    }
    if (WIFCONTINUED(status))
    {
        // child is continued, set it to running
        block_signal(SIGCHLD, NULL);
        set_child_status(child, PSTATE_RUNNING, -1);
        child_summary(child, STDOUT_FILENO);
        unblock_signal(SIGCHLD, NULL);
        return;
    }
    }
}

int suspend_until_state(int deetId, int state)
{
    child_t* child = get_child_by_deet_id(deetId);
    if (child == NULL)
        return -1;
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);
    sigaddset(&mask, SIGCONT);
    sigaddset(&mask, SIGKILL);
    while (1)
    {
        ptrace(PTRACE_INTERRUPT, child->pid, NULL, NULL);
        if (child->status == state)
        {
            kill(getpid(), SIGCHLD);
            return 0;
        }
        // block_signal(SIGCHLD, NULL);
        unblock_signal(SIGCHLD, NULL);
        ptrace(PTRACE_DETACH, child->pid, NULL, NULL);
        sigsuspend(&oldmask);
    }
}

void handle_sigint(int sig)
{
    shutdown = 1;
    log_signal(sig);
    unblock_signal(SIGCHLD, NULL);
    kill(getpid(), SIGCHLD);
}

void block_signal(int sig, sigset_t* old)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_BLOCK, &mask, old);
}

void unblock_signal(int sig, sigset_t* old)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_UNBLOCK, &mask, old);
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