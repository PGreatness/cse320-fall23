#include "signaling.h"
#include "track_children.h"

int shutdown = 0;
int eof_flag = 0;
int free_flag = 1;

sigset_t block_all_signals()
{
    sigset_t mask, oldmask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    return oldmask;
}

sigset_t reset_signals(sigset_t* oldmask)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, oldmask, NULL);
    return mask;
}

void handle_shutdown(int sig)
{
    if (!shutdown)
        return;
    child_t* s = sentinel.next;
    info("Shutting down");
    sigset_t mask;
    while (s != &sentinel)
    {
        if (s->status == PSTATE_DEAD)
        {
            s = s->next;
            continue;
        }
        mask = block_all_signals();
        kill(s->pid, SIGKILL);
        set_child_status(s, PSTATE_KILLED, -1);
        // child_summary(s, STDOUT_FILENO);
        reset_signals(&mask);
        s = s->next;
    }

    child_summaries_in_state(PSTATE_KILLED, STDOUT_FILENO);
    // stops the program from relooping and attempting to kill the children again
    shutdown = 0;

    sigset_t mask2;
    sigemptyset(&mask2);
    sigaddset(&mask2, SIGINT);

    while (count_in_state(PSTATE_KILLED) > 0) sigsuspend(&mask2);

    // only the dead children remain
    // free them
    s = sentinel.next;
    while (s != &sentinel)
    {
        child_t* next = s->next;
        free_child(s);
        s = next;
    }
    info("All children dead");
    log_shutdown();
    exit(EXIT_SUCCESS);
}

void handle_sigchild(int sig)
{
    handle_shutdown(sig);
    int status;
    pid_t child_pid;
    sigset_t block;
    while ((child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {
        can_print = 0;
        log_signal(sig);
        debug("Signal recieved, is we done? %s, child: %i", WIFEXITED(status) ? "yes" : "no", child_pid);
        child_t *child = get_child(child_pid);
        if (child == NULL)
        {
            debug("child not found");
            return;
        }
        if (WTERMSIG(status) == SIGKILL)
        {
            // child is killed, set it to killed
            block = block_all_signals();
            // block_signal(SIGCHLD, NULL);
            set_exit_status(child, SIGKILL);
            set_child_status(child, PSTATE_DEAD, status);
            child_summary(child, STDOUT_FILENO);
            // unblock_signal(SIGCHLD, NULL);
            reset_signals(&block);
            kill(getpid(), SIGCHLD);
        }
        if (WIFEXITED(status))
        {
            // child is exited, set it to dead
            // check the child's exit status
            // block_signal(SIGCHLD, NULL);
            block = block_all_signals();
            // int exit_status = WEXITSTATUS(status);
            set_child_status(child, PSTATE_DEAD, status);
            child_summary(child, STDOUT_FILENO);
            decrement_next_deet_id();
            // unblock_signal(SIGCHLD, NULL);
            reset_signals(&block);
            kill(getpid(), SIGCHLD);
        }
        if (WIFSTOPPED(status))
        {
            // block_signal(SIGCHLD, NULL);
            block = block_all_signals();
            set_child_status(child, PSTATE_STOPPED, -1);
            child_summary(child, STDOUT_FILENO);
            // unblock_signal(SIGCHLD, NULL);
            reset_signals(&block);
        }
        if (WIFCONTINUED(status))
        {
            // child is continued, set it to running
            // block_signal(SIGCHLD, NULL);
            block = block_all_signals();
            set_child_status(child, PSTATE_RUNNING, -1);
            child_summary(child, STDOUT_FILENO);
            // unblock_signal(SIGCHLD, NULL);
            reset_signals(&block);
        }
        can_print = 1;
    }
}

int suspend_until_state(int deetId, int state)
{
    child_t* child = get_child_by_deet_id(deetId);
    if (child == NULL)
        return -1;
    sigset_t mask;
    sigemptyset(&mask);
    free_flag = 0;
    while (1)
    {
        if (child->status == state)
        {
            free_flag = 1;
            return 0;
        }
        sigsuspend(&mask);
    }
}

void handle_sigint(int sig)
{
    shutdown = 1;
    log_signal(sig);
    // free the getline buffer
    if (free_flag) free(getline_buffer);
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