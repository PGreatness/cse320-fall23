#ifndef HW5_STUDENT_CSAPP_H_
#define HW5_STUDENT_CSAPP_H_
/* $begin csapp.h */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "debug.h"

/* Simplifies calls to bind(), connect(), and accept() */
/* $begin sockaddrdef */
typedef struct sockaddr SA;
/* $end sockaddrdef */

/* Misc constants */
#define MAXLINE   8192  /* Max text line length */
#define MAXBUF    8192  /* Max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

/* Sockets interface wrappers */
int Socket(int domain, int type, int protocol);
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void Listen(int s, int backlog);
int Accept(int s, struct sockaddr *addr, unsigned int *addrlen);
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

/* DNS wrappers */
struct hostent *Gethostbyname(const char *name);
struct hostent *Gethostbyaddr(const char *addr, int len, int type);

/* Stevens's socket I/O functions (UNP, Sec 3.9) */
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen); /* non-reentrant */

/* 
 * Stevens's reentrant readline_r package
 */
/* struct used by readline_r */
/* $begin rline */
typedef struct {
    int read_fd;	/* caller's descriptor to read from */
    char *read_ptr;	/* caller's buffer to read into */
    size_t read_maxlen;	/* max bytes to read */

    /* next three are used internally by the function */
    int rl_cnt;		/* initialize to 0 */
    char *rl_bufptr;	/* initialize to rl_buf */
    char rl_buf[MAXBUF];/* internal buffer */
} Rline;
/* $end rline */

void readline_rinit(int fd, void *ptr, size_t maxlen, Rline *rptr);
ssize_t	readline_r(Rline *rptr);

/* Wrappers for Stevens's socket I/O helpers */
ssize_t Readn(int fd, void *vptr, size_t n);
void Writen(int fd, void *vptr, size_t n);
ssize_t Readline(int fd, void *vptr, size_t maxlen);
ssize_t	Readline_r(Rline *);
void Readline_rinit(int fd, void *ptr, size_t maxlen, Rline *rptr);

/* Our own client/server helper functions */
int open_clientfd(char *hostname, int portno);
int open_listenfd(int portno);
void Close(int fd);
/* $end csapp.h */


#endif // HW5_STUDENT_CSAPP_H_