#ifndef HW4_SIGNALING_H
#define HW4_SIGNALING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// Handlers
/**
 * Signal handler for SIGCHLD
 * @param sig The signal number
 */
void handle_sigchild(int sig);

void block_signal(int sig);
void unblock_signal(int sig);
void handle_signal_using_handler(int sig, void (*handler)(int));


#endif //HW4_SIGNALING_H