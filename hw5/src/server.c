#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"
#include "transaction.h"
#include "store.h"
#include "protocol.h"
#include "student/csapp.h"
#include "debug.h"

CLIENT_REGISTRY *client_registry;

void send_reply_packet(int fd, XACTO_PACKET* request, int status)
{
    XACTO_PACKET *response = malloc(sizeof(XACTO_PACKET));
    if (response == NULL) {
        debug("response is null");
        return;
    }
    response->type = XACTO_REPLY_PKT;
    response->serial = request->serial;
    response->size = 0;
    response->status = status;
    response->timestamp_sec = request->timestamp_sec;
    response->timestamp_nsec = request->timestamp_nsec;

    // convert to network byte order
    response->serial = htonl(response->serial);
    response->size = htonl(response->size);
    response->timestamp_sec = htonl(response->timestamp_sec);
    response->timestamp_nsec = htonl(response->timestamp_nsec);

    int write_success = proto_send_packet(fd, response, NULL);
    if (write_success < 0) {
        debug("Error writing response packet");
        return;
    }
    // free the response packet
    free(response);
}

void send_key_packet(int fd, XACTO_PACKET* request, void* payload, int status)
{
    XACTO_PACKET *response = malloc(sizeof(XACTO_PACKET));
    if (response == NULL) {
        debug("response is null");
        return;
    }
    response->type = XACTO_KEY_PKT;
    response->serial = request->serial;
    response->size = request->size;
    response->status = status;
    response->timestamp_sec = request->timestamp_sec;
    response->timestamp_nsec = request->timestamp_nsec;

    // convert to network byte order
    response->serial = htonl(response->serial);
    response->size = htonl(response->size);
    response->timestamp_sec = htonl(response->timestamp_sec);
    response->timestamp_nsec = htonl(response->timestamp_nsec);

    int write_success = proto_send_packet(fd, response, payload);
    if (write_success < 0) {
        debug("Error writing response packet");
        return;
    }
    // free the response packet
    free(response);
}

void send_value_packet(int fd, XACTO_PACKET* request, void* payload, int size, int status)
{
    XACTO_PACKET *response = malloc(sizeof(XACTO_PACKET));
    if (response == NULL) {
        debug("response is null");
        return;
    }
    response->type = XACTO_VALUE_PKT;
    response->serial = request->serial;
    response->size = size;
    response->status = status;
    response->timestamp_sec = request->timestamp_sec;
    response->timestamp_nsec = request->timestamp_nsec;

    // convert to network byte order
    response->serial = htonl(response->serial);
    response->size = htonl(response->size);
    response->timestamp_sec = htonl(response->timestamp_sec);
    response->timestamp_nsec = htonl(response->timestamp_nsec);

    int write_success = proto_send_packet(fd, response, payload);
    if (write_success < 0) {
        debug("Error writing response packet");
        return;
    }
    // free the response packet
    free(response);
}

void *xacto_client_service(void *arg)
{
    int fd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());
    debug("xacto_client_service started for fd %d", fd);
    // register the client
    creg_register(client_registry, fd);
    // create a transaction for the client
    TRANSACTION *client_transaction = trans_create();
    if (client_transaction == NULL) {
        debug("client transaction is null\n");
        return NULL;
    }
    int type = XACTO_NO_PKT;
    BLOB *tmp = (BLOB*)NULL;
    KEY *key;
    BLOB *value;
    TRANS_STATUS transaction_stat;
    int send_packet = 0;
    // enter a loop to read requests from the client file descriptor
    while (1)
    {
        XACTO_PACKET *request = malloc(sizeof(XACTO_PACKET));
        if (request == NULL) {
            debug("request is null");
            return NULL;
        }
        void **payload = malloc(sizeof(void *));
        if (payload == NULL) {
            debug("payload is null");
            // free the request packet
            free(request);
            return NULL;
        }
        // read the request packet
        debug("Reading request packet");
        int read_success = proto_recv_packet(fd, request, payload);
        if (read_success < 0) {
            debug("Error reading request packet");
            // free the request packet
            free(request);
            // free the payload
            free(payload);
            // unregister the client
            creg_unregister(client_registry, fd);
            return NULL;
        }
        // convert to host byte order
        request->serial = ntohl(request->serial);
        request->size = ntohl(request->size);
        request->timestamp_sec = ntohl(request->timestamp_sec);
        request->timestamp_nsec = ntohl(request->timestamp_nsec);
        // handle the request
        switch (request->type)
        {
            case XACTO_GET_PKT:
                debug("Received GET request");
                if (type != XACTO_NO_PKT) {
                    debug("Error: transaction already in progress");
                    return NULL;
                }
                type = XACTO_GET_PKT;
                break;
            case XACTO_PUT_PKT:
                debug("Received PUT request");
                if (type != XACTO_NO_PKT)
                {
                    debug("Error: transaction already in progress");
                    return NULL;
                }
                type = XACTO_PUT_PKT;
                break;
            case XACTO_COMMIT_PKT:
                debug("Received COMMIT request");
                if (type != XACTO_NO_PKT)
                {
                    debug("Error; transaction already in progress");
                    return NULL;
                }
                type = XACTO_COMMIT_PKT;
                // TODO: finish commiting and return
                if ((transaction_stat = trans_commit(client_transaction)) == TRANS_ABORTED)
                {
                    debug("Error: transaction aborted");
                }else{
                    debug("Transaction committed");
                }
                send_packet = 1;
                break;
            case XACTO_KEY_PKT:
                debug("Received a key");
                if (type == XACTO_NO_PKT)
                {
                    debug("Error: no transaction in progress");
                    return NULL;
                }
                if (payload == NULL)
                {
                    debug("Error: no key payload");
                    return NULL;
                }
                tmp = blob_create(*payload, request->size);
                if (tmp == NULL)
                {
                    debug("Error: blob_create failed");
                    return NULL;
                }
                key = key_create(tmp);
                if (key == NULL)
                {
                    debug("Error: key_create failed");
                    return NULL;
                }
                if (type == XACTO_GET_PKT)
                {
                    transaction_stat = store_get(client_transaction, key, &value);
                    if (transaction_stat == TRANS_ABORTED)
                    {
                        debug("Error with get: transaction aborted");
                    }
                    send_packet = 1;
                }
                break;
            case XACTO_VALUE_PKT:
                debug("Received a value");
                if (type == XACTO_NO_PKT)
                {
                    debug("Error: no transaction in progress");
                    return NULL;
                }
                if (payload == NULL)
                {
                    debug("Error: no value payload");
                    return NULL;
                }
                tmp = blob_create(*payload, request->size);
                if (tmp == NULL)
                {
                    debug("Error: blob_create failed");
                    return NULL;
                }
                if (type == XACTO_GET_PKT)
                {
                    // get the value from the store
                    value = malloc(sizeof(BLOB));
                    if (value == NULL)
                    {
                        debug("Error: malloc failed");
                        return NULL;
                    }
                    debug("Getting value from store");
                    transaction_stat = store_get(client_transaction, key, &value);
                    if (transaction_stat == TRANS_ABORTED)
                    {
                        debug("Error: transaction aborted");
                    }
                }
                if (type == XACTO_PUT_PKT)
                {
                    value = tmp;
                    // put the value in the store
                    transaction_stat = store_put(client_transaction, key, value);
                    if (transaction_stat == TRANS_ABORTED)
                    {
                        debug("Error: transaction aborted");
                    }
                }
                send_packet = 1;
                break;
            default:
                debug("Error: invalid request type");
                return NULL;
        }

        if (!send_packet) continue;
        send_reply_packet(fd, request, transaction_stat);
        if (type == XACTO_GET_PKT)
        {
            info("value got: %s", value->content);
            send_value_packet(fd, request, value->content, strlen(value->content), transaction_stat);
        }
        // free the request packet
        free(request);
        // reset the type
        type = XACTO_NO_PKT;
        // reset the send_packet flag
        send_packet = 0;
        // reset the key and value
        key = NULL;
        value = NULL;
        // reset the tmp blob
        tmp = NULL;
        if (transaction_stat == TRANS_ABORTED)
        {
            debug("Error: transaction aborted");
            return NULL;
        }
        if (transaction_stat == TRANS_COMMITTED)
        {
            debug("transaction committed, can exit loop");
            creg_unregister(client_registry, fd);
            break;
        }
    }
    return NULL;
}