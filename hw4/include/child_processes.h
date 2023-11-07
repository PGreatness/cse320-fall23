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

void show_child_process(pid_t pid, FILE* stream);

int continue_child_process(int deet_id);

int kill_child_process(int deet_id);


#endif //HW4_CHILD_PROCESSES_H