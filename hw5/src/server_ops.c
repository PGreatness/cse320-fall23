#include "student/server_ops.h"
#include "server.h"

int server_sockfd;

int listen_for_connections(int port)
{
    int listenfd, *connfdp;
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