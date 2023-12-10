#ifndef HW5_STUDENT_THREAD_OPS_H_
#define HW5_STUDENT_THREAD_OPS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "student/csapp.h"
#include "debug.h"

void create_thread(pthread_t *thread, pthread_attr_t *attrp, void *(*start_routine) (void *), void *arg);

void process_connection(int connfd);

#endif // HW5_STUDENT_THREAD_OPS_H_