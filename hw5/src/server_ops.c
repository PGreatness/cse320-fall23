#include "student/server_ops.h"

struct args {
    CLIENT_REGISTRY *cr;
    int fd;
};

int listen_for_connections(int port)
{
    int *connfdp;
    int listenfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = open_listenfd(port);
    while (1) {
        debug("listening for connections");
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        debug("connection accepted on fd %i", *connfdp);
        if (*connfdp < 0) {
            free(connfdp);
            debug("connection failed");
            break;
        }
        // create_thread(&tid, NULL, &xacto_client_service, connfdp);
        pthread_create(&tid, NULL, &xacto_client_service, connfdp);
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
        pthread_create(&tid[i], NULL, &create_client, arg);
    }
    // client registry should have num_threads clients
    // with each thread registering a client and running detached
    // so we should have num_threads clients
    creg_wait_for_empty(cr);
    return 0;
}
