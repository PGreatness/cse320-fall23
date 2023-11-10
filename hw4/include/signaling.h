#ifndef HW4_SIGNALING_H
#define HW4_SIGNALING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

extern int shutdown;
extern int eof_flag;

// Handlers
/**
 * Signal handler for SIGCHLD
 * @param sig The signal number
 */
void handle_sigchild(int sig);

void handle_sigint(int sig);

void block_signal(int sig, sigset_t *set);
void unblock_signal(int sig, sigset_t *set);

int suspend_until_state(int deetId, int state);

void handle_signal_using_handler(int sig, void (*handler)(int));


#endif //HW4_SIGNALING_H