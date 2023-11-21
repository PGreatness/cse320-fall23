#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "protocol.h"
#include "debug.h"

int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data)
{
    // Convert multi-byte fields to network byte order
    pkt->size = htonl(pkt->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);
    pkt->null = data == NULL ? 1 : 0;

    // Send the packet
    debug("Sending packet");
    int sent = write(fd, pkt, sizeof(XACTO_PACKET));
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
        sent = write(fd, data, pkt->size);
        if (sent < 0)
        {
            debug("Error sending payload");
            errno = EIO;
            return -1;
        }
    } else {
        // Send a special null data value
        uint8_t null_data = 0;
        sent = write(fd, &null_data, 1);
        if (sent < 0)
        {
            debug("Error sending null data");
            errno = EIO;
            return -1;
        }
    }

    return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap)
{
    // Receive the packet
    debug("Receiving packet");
    int received = read(fd, pkt, sizeof(XACTO_PACKET));
    if (received < 0)
    {
        debug("Error receiving packet");
        errno = EIO;
        return -1;
    }

    // Convert multi-byte fields to host byte order
    pkt->size = ntohl(pkt->size);
    pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
    pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);

    debug("Packet: type: %d, status: %d, null: %d, size: %d, timestamp_sec: %d, timestamp_nsec: %d", pkt->type, pkt->status, pkt->null, pkt->size, pkt->timestamp_sec, pkt->timestamp_nsec);
    // Receive the payload, if any
    if (pkt->null == 1)
    {
        *datap = malloc(pkt->size);
        if (*datap == NULL)
        {
            errno = ENOMEM;
            return -1;
        }

        debug("Receiving payload");
        received = read(fd, *datap, pkt->size);
        if (received < 0)
        {
            debug("Error receiving payload");
            errno = EIO;
            return -1;
        }
    } else {
        // Receive a special null data value
        /* uint8_t null_data;
        received = read(fd, &null_data, sizeof(uint8_t));
        if (received < 0)
        {
            debug("Error receiving null data");
            errno = EIO;
            return -1;
        } */
    }

    return 0;
}