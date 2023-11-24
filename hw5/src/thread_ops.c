#include "student/thread_ops.h"
#include "protocol.h"
#include <errno.h>

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
    XACTO_PACKET *pkt = malloc(sizeof(XACTO_PACKET));
    void *data = NULL;
    int rc;
    if ((rc = proto_recv_packet(connfd, pkt, &data)) < 0) {
        debug("error: proto_recv_packet, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    warn("Received packet with type: %d and serial: %d\n", pkt->type, pkt->serial);
    warn("Buffer: %s\n", (char *)data);
    // send a reply
    XACTO_PACKET *reply = malloc(sizeof(XACTO_PACKET));
    reply->type = XACTO_REPLY_PKT;
    reply->status = 0;
    reply->null = 0;
    reply->size = 0;
    reply->timestamp_sec = 0;
    reply->timestamp_nsec = 0;
    reply->serial = pkt->serial;
    if ((rc = proto_send_packet(connfd, reply, NULL)) < 0) {
        debug("error: proto_send_packet, rc: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    debug("Sent reply packet\n");
}