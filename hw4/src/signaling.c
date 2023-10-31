#include "signaling.h"

void sigchild_handler(int sig)
{
    // test signal handler
    fprintf(stderr,"SIGCHLD received.\n");
    fflush(stdin);
}

void block_signal(int sig)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    fflush(stdin);
}

void unblock_signal(int sig)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    fflush(stdin);
}

void handle_signal(int sig, void (*handler)(int))
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(sig, &sa, NULL) == -1)
    {
        fprintf(stderr, "Error: sigaction() failed.\n");
        exit(EXIT_FAILURE);
    }
    fflush(stdin);
}