#include "student/server_ops.h"
#include "server.h"

int server_sockfd;
int *connfdp;

struct args {
    CLIENT_REGISTRY *cr;
    int fd;
};

int listen_for_connections(int port)
{
    int listenfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = open_listenfd(port);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        create_thread(&tid, NULL, xacto_client_service, connfdp);
    }
    Close(listenfd);
    return 0;
}

void *create_client(void *arg)
{
    // arg is a struct with a client registry and a file descriptor
    int fd = ((struct args *)arg)->fd;
    CLIENT_REGISTRY *clr = ((struct args *)arg)->cr;
    free(arg);
    pthread_detach(pthread_self());
    creg_register(clr, fd);
    sleep(1);
    creg_unregister(clr, fd);
    return NULL;

}

int test_client_registry(CLIENT_REGISTRY *cr)
{
    // test the client registry by rapidly registering and unregistering clients
    // and checking that the number of clients is correct
    int i;
    int fd;
    // int num_clients = 0;
    int num_threads = 5;
    pthread_t tid[num_threads];
    for (i = 0; i < num_threads; i++) {
        fd = i;
        struct args *arg = malloc(sizeof(struct args));
        arg->cr = cr;
        arg->fd = fd;
        debug("creating thread %d with fd of %i\n", i, fd);
        create_thread(&tid[i], NULL, create_client, arg);
    }
    // client registry should have num_threads clients
    // with each thread registering a client and running detached
    // so we should have num_threads clients
    creg_wait_for_empty(cr);
    return 0;
}

/* void *xacto_client_service(void *arg)
{
    int connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());
    process_connection(connfd);
    int rc;
    if ((rc = close(connfd)) < 0) {
        debug("error: close, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    return NULL;
} */