#ifndef HW5_STUDENT_SERVER_H_
#define HW5_STUDENT_SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"
#include "student/csapp.h"
#include "student/thread_ops.h"
#include "client_registry.h"

int listen_for_connections(int port);

int test_client_registry(CLIENT_REGISTRY *cr);

#endif // HW5_STUDENT_SERVER_H_