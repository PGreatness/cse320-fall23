#include "student/thread_ops.h"

void create_thread(pthread_t *thread, pthread_attr_t *attrp, void *(*start_routine) (void *), void *arg)
{
    int rc;
    if ((rc = pthread_create(thread, attrp, start_routine, arg)) != 0) {
        debug("error: pthread_create, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

void process_connection(int connfd)
{
    debug("Connection established with client with connfd: %d\n", connfd);
    // read what the client has to say
    char buffer[1024];
    int n = read(connfd, buffer, 1024);
    if (n < 0) {
        debug("error: read, n: %d\n", n);
        exit(EXIT_FAILURE);
    }
    debug("Client message: %s\n", buffer);
}