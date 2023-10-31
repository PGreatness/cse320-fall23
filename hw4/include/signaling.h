#ifndef HW4_SIGNALING_H
#define HW4_SIGNALING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sigchild_handler(int sig);
void block_signal(int sig);
void unblock_signal(int sig);
void handle_signal(int sig, void (*handler)(int));


#endif //HW4_SIGNALING_H