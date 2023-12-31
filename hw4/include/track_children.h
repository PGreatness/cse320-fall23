#ifndef HW4_TRACK_CHILDREN_H
#define HW4_TRACK_CHILDREN_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>

#include "deet.h"
#include "debug.h"
#include "util.h"

typedef struct child {
    pid_t pid;
    pid_t deetId;
    int status;
    int trace;
    int exit_status;
    char *command;
    char **args;
    int argc;
    struct child *next;
    struct child *prev;
} child_t;

extern child_t sentinel;

/**
 * @brief Spawn a child process.
 * @param pid The PID of the child process.
 * @param trace Whether or not to trace the child process.
 * @param args The arguments to pass to the command.
 * @param num_args The number of arguments.
 * @return A pointer to the `child_t` struct for the child process.
*/
child_t *spawn_child(pid_t pid, int trace, char *args[], int num_args);
/**
 * @brief Kill a child process.
 * @param pid The PID of the child process.
 * @return The PID of the killed child process.
*/
pid_t kill_child(pid_t pid);
/**
 * @brief Remove all dead child processes.
 * @return The number of dead child processes removed.
*/
int remove_all_dead_children();

/**
 * @brief Get the child process with the given PID.
 * @param pid The PID of the child process.
 * @return A pointer to the `child_t` struct for the child process.
*/
child_t *get_child(pid_t pid);

/**
 * @brief Get the number of child processes in the given state.
 * @param state The state to check for.
 * @return The number of child processes in the given state.
*/
int count_in_state(int state);
/**
 * @brief Get the status of the child process with the given PID.
 * @param pid The PID of the child process.
 * @return The status of the child process.
*/
int get_child_status(pid_t pid);
/**
 * @brief Set the status of the child process with the given PID.
 * @param pid The PID of the child process.
 * @param status The status of the child process.
 * @return 0 if successful, -1 otherwise.
*/
int set_child_status_by_pid(pid_t pid, int status);

/**
 * @brief Set the status of the child process.
 * @param child A pointer to the `child_t` struct for the child process.
 * @param status The status of the child process.
 * @param exit_status The exit status of the child process. Can be 0 if
 * the child process did not exit.
 * @return 0 if successful, -1 otherwise.
*/
int set_child_status(child_t *child, int status, int exit_status);

/**
 * @brief Get the child process with the given deet ID.
 * @param deetId The deet ID of the child process.
 * @return A pointer to the `child_t` struct for the child process.
*/
child_t *get_child_by_deet_id(pid_t deetId);

child_t *get_child_by_deet_id_ig(pid_t deetId);

/**
 * @brief Get the last child process.
 * @return A pointer to the `child_t` struct for the last child process.
*/
child_t *get_last_child();

pid_t stop_child(pid_t pid);

pid_t release_child(pid_t pid);

void child_summaries_in_state(int state, int filenum);

void child_summary(child_t* child, int filenum);

void set_exit_status(child_t *child, int status);

void free_child(child_t *child);

int decrement_next_deet_id();
int increment_next_deet_id();
int get_next_deet_id();

#endif //HW4_TRACK_CHILDREN_H