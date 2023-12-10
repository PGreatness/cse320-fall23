#include "student/signaling.h"
#include "debug.h"

sigset_t block_signal(int signum)
{
    sigset_t current_mask;
    sigset_t new_mask;
    sigprocmask(SIG_SETMASK, NULL, &current_mask);
    sigprocmask(SIG_SETMASK, NULL, &new_mask);
    sigaddset(&new_mask, signum);
    sigprocmask(SIG_SETMASK, &new_mask, NULL);
    return current_mask;
}

sigset_t unblock_signal(int signum)
{
    sigset_t current_mask;
    sigset_t new_mask;
    sigprocmask(SIG_SETMASK, NULL, &current_mask);
    sigprocmask(SIG_SETMASK, NULL, &new_mask);
    sigdelset(&new_mask, signum);
    sigprocmask(SIG_SETMASK, &new_mask, NULL);
    return current_mask;
}

sigset_t block_all_signals()
{
    sigset_t current_mask;
    sigset_t new_mask;
    sigprocmask(SIG_SETMASK, NULL, &current_mask);
    sigprocmask(SIG_SETMASK, NULL, &new_mask);
    sigfillset(&new_mask);
    sigprocmask(SIG_SETMASK, &new_mask, NULL);
    return current_mask;
}

sigset_t unblock_all_signals()
{
    sigset_t current_mask;
    sigset_t new_mask;
    sigprocmask(SIG_SETMASK, NULL, &current_mask);
    sigprocmask(SIG_SETMASK, NULL, &new_mask);
    sigemptyset(&new_mask);
    sigprocmask(SIG_SETMASK, &new_mask, NULL);
    return current_mask;
}

sigset_t reset_signal_mask(sigset_t *old_mask)
{
    sigset_t current_mask;
    sigprocmask(SIG_SETMASK, NULL, &current_mask);
    sigprocmask(SIG_SETMASK, old_mask, NULL);
    return current_mask;
}