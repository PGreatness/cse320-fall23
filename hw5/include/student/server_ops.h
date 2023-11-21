#ifndef HW5_STUDENT_SERVER_H_
#define HW5_STUDENT_SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "student/csapp.h"
#include "student/thread_ops.h"

extern int server_sockfd;


int listen_for_connections(int port);

#endif // HW5_STUDENT_SERVER_H_