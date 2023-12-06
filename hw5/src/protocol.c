#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "student/csapp.h"
#include "protocol.h"
#include "debug.h"

int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data)
{
    // Convert multi-byte fields to network byte order
    /* debug("serial: %d", htonl(pkt->serial));
    pkt->serial = htonl(pkt->serial);
    pkt->size = htonl(pkt->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec); */
    pkt->null = data == NULL ? 1 : 0;

    // Send the packet
    debug("Sending packet:\ntype: %d,\nstatus: %d,\nnull: %d,\nserial: %d,\nsize: %d,\ntimestamp_sec: %d,\ntimestamp_nsec: %d",
        pkt->type, pkt->status, pkt->null, htonl(pkt->serial), htonl(pkt->size), htonl(pkt->timestamp_sec), htonl(pkt->timestamp_nsec));
    // int sent = write(fd, pkt, sizeof(XACTO_PACKET));
    int sent = writen(fd, pkt, sizeof(XACTO_PACKET));
    if (sent < 0)
    {
        debug("Error sending packet");
        errno = EIO;
        return -1;
    }

    // Send the payload, if any
    if (data != NULL)
    {
        debug("Sending payload");
        // sent = write(fd, data, htonl(pkt->size));
        sent = writen(fd, data, htonl(pkt->size));
        if (sent < 0)
        {
            debug("Error sending payload [%s] to fd %d of size %d", (char *)data, fd, htonl(pkt->size));
            // perror("write");
            errno = EIO;
            return -1;
        }
        debug("sent payload of size %d", htonl(pkt->size));
    } else {
        debug("Send a special null data value, which has no content");
        sent = writen(fd, NULL, 0);
        if (sent < 0)
        {
            debug("Error sending payload");
            errno = EIO;
            return -1;
        }
        debug("sent payload of size %d", 0);
        // sent = write(fd, NULL, 0);
        /* XACTO_PACKET xdata = {0};
        xdata.null = 1;
        sent = writen(fd, &xdata, sizeof(XACTO_PACKET));
        if (sent < 0)
        {
            debug("Error sending payload");
            perror("write");
            errno = EIO;
            return -1;
        }
        debug("sent payload of size %li", sizeof(XACTO_PACKET)); */
    }
    debug("Sent packet");

    return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap)
{
    // Receive the packet
    debug("Receiving packet");
    // int received = read(fd, pkt, sizeof(XACTO_PACKET));
    int received = readn(fd, pkt, sizeof(XACTO_PACKET));
    if (received < 0)
    {
        debug("Error receiving packet");
        // perror("read");
        errno = EIO;
        return -1;
    }
    // check for timeout
    if (ntohl(pkt->size) < 0 || ntohl(pkt->size) > sizeof(XACTO_PACKET))
    {
        debug("Timeout received");
        errno = ETIMEDOUT;
        return -1;
    }
    // check for eof
    if (received == 0)
    {
        debug("EOF received");
        // close the connection
        Close(fd);
        return -1;
    }

    // Convert multi-byte fields to host byte order
    // pkt->serial = ntohl(pkt->serial);
    // pkt->size = ntohl(pkt->size);
    // pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
    // pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);

    debug("Received packet:\ntype: %d,\nstatus: %d,\nnull: %d,\nserial: %d,\nsize: %d,\ntimestamp_sec: %d,\ntimestamp_nsec: %d",
        pkt->type, pkt->status, pkt->null, ntohl(pkt->serial), ntohl(pkt->size), ntohl(pkt->timestamp_sec), ntohl(pkt->timestamp_nsec));

    if (ntohl(pkt->size))
    {
        // Receive the key
        uint32_t size = ntohl(pkt->size);
        debug("Receiving key of size %d", size);
        if (size > 0)
        {
            *datap = malloc(size + 1);
            if (*datap == NULL)
            {
                debug("Error allocating memory for payload");
                errno = ENOMEM;
                return -1;
            }
            // received = read(fd, *datap, size);
            received = readn(fd, *datap, size);
            if (received < 0)
            {
                debug("Error receiving payload");
                errno = EIO;
                return -1;
            }
            *((char *)*datap + size) = '\0';
        }
        debug("Received key: %s", (char *)*datap);
    }
    return 0;
}