#ifndef HW4_FREE_USED_H
#define HW4_FREE_USED_H

#include <stdlib.h>
#include <semaphore.h>
#include "debug.h"
#include "track_children.h"
#include "child_processes.h"

void free_args(char *args[], int size);

void free_children();

#endif //HW4_FREE_USED_H