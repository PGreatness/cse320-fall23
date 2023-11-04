#ifndef HW4_TRACK_CHILDREN_H
#define HW4_TRACK_CHILDREN_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "deet.h"
#include "debug.h"

typedef struct child {
    pid_t pid;
    pid_t deetId;
    int status;
    int trace;
    char *command;
    char **args;
    struct child *next;
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
 * @return 0 if successful, -1 otherwise.
*/
int set_child_status(child_t *child, int status);

/**
 * @brief Get the child process with the given deet ID.
 * @param deetId The deet ID of the child process.
 * @return A pointer to the `child_t` struct for the child process.
*/
child_t *get_child_by_deet_id(pid_t deetId);

/**
 * @brief Get the last child process.
 * @return A pointer to the `child_t` struct for the last child process.
*/
child_t *get_last_child();

#endif //HW4_TRACK_CHILDREN_H