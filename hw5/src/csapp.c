#include "student/csapp.h"
#include "errno.h"

/*
 * open_listenfd - open and return a listening socket on port
 */
/* $begin open_listenfd */
int open_listenfd(int port)
{
    int listenfd;
    int optval;
    struct sockaddr_in serveraddr;

    /* create a socket descriptor */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* eliminates "Address already in use" error from bind. */
    optval = 1;
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
	       (const void *)&optval , sizeof(int));

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    Bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr));

    /* make it a listening socket ready to accept connection requests */
    Listen(listenfd, LISTENQ);

    return listenfd;
}
/* $end open_listenfd */


int Socket(int domain, int type, int protocol)
{
    int rc;

    if ((rc = socket(domain, type, protocol)) < 0)
    {
        debug("error: socket, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    return rc;
}

void Bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    int rc;

    if ((rc = bind(sockfd, my_addr, addrlen)) < 0)
    {
        debug("error: bind, rc: %d\n", rc);
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void Listen(int s, int backlog)
{
    int rc;

    if ((rc = listen(s,  backlog)) < 0)
    {
        debug("error: listen, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

int Accept(int s, struct sockaddr *addr, unsigned int *addrlen)
{
    int rc;

    if ((rc = accept(s, addr, addrlen)) < 0)
    {
        debug("error: accept, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    return rc;
}

void Setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
    int rc;

    if ((rc = setsockopt(s, level, optname, optval, optlen)) < 0)
    {
        debug("error: setsockopt, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}