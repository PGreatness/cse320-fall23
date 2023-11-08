#ifndef HW4_CHILD_PROCESSES_H
#define HW4_CHILD_PROCESSES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#include "deet.h"
#include "track_children.h"
#include "signaling.h"

// Function prototypes
int run_child_process(char* command, char* args[], int num_args);

void show_child_process(pid_t pid, int filenum);

void show_all_child_processes(int filenum);

int continue_child_process(int deet_id, int filenum);

int kill_child_process(int deet_id);

void kill_all_child_processes();


#endif //HW4_CHILD_PROCESSES_H