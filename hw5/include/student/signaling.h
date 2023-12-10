#ifndef HW5_STUDENT_SIGNALING_H
#define HW5_STUDENT_SIGNALING_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

sigset_t block_signal(int signum);

sigset_t unblock_signal(int signum);

sigset_t block_all_signals();

sigset_t unblock_all_signals();

sigset_t reset_signal_mask(sigset_t *old_mask);

void install_signal_handler(int signum, void (*handler)(int), int flags);

void sighup_handler(int signum);

#endif // HW5_STUDENT_SIGNALING_H