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
    memset(response, 0, sizeof(XACTO_PACKET));
    response->type = XACTO_REPLY_PKT;
    response->serial = request->serial;
    response->size = 0;
    response->null = 1;
    response->status = status;
    response->timestamp_sec = request->timestamp_sec;
    response->timestamp_nsec = request->timestamp_nsec;

    // convert to network byte order
    response->serial = htonl(response->serial);
    response->size = htonl(response->size);
    response->timestamp_sec = htonl(response->timestamp_sec);
    response->timestamp_nsec = htonl(response->timestamp_nsec);

    int write_success = proto_send_packet(fd, response, NULL);
    free(response);
    if (write_success < 0) {
        debug("Error writing response packet");
        return;
    }
}

void send_value_packet(int fd, XACTO_PACKET* request, void* payload, int size, int status)
{
    XACTO_PACKET *response = malloc(sizeof(XACTO_PACKET));
    if (response == NULL) {
        debug("response is null");
        return;
    }
    memset(response, 0, sizeof(XACTO_PACKET));
    response->type = XACTO_VALUE_PKT;
    response->serial = request->serial;
    response->size = size;
    response->null = 0;
    response->status = status;
    response->timestamp_sec = request->timestamp_sec;
    response->timestamp_nsec = request->timestamp_nsec;

    // convert to network byte order
    response->serial = htonl(response->serial);
    response->size = htonl(response->size);
    response->timestamp_sec = htonl(response->timestamp_sec);
    response->timestamp_nsec = htonl(response->timestamp_nsec);

    int write_success = proto_send_packet(fd, response, payload);
    free(response);
    if (write_success < 0) {
        debug("Error writing response packet");
        return;
    }
}

void *xacto_client_service(void *arg)
{
    int fd = *(int *)arg;
    free(arg);
    pthread_detach(pthread_self());
    debug("xacto_client_service: fd %d", fd);
    if (creg_register(client_registry, fd) < 0)
    {
        debug("Error registering client");
        return NULL;
    }
    TRANSACTION *cl_trans = trans_create();
    debug("Created a transaction for client %d", fd);
    if (cl_trans == NULL)
    {
        debug("Error creating transaction");
        creg_unregister(client_registry, fd);
        return NULL;
    }
    int read_key_success;
    int read_value_success;
    while (1)
    {
        XACTO_PACKET *req_pkt = malloc(sizeof(XACTO_PACKET));
        if (req_pkt == NULL)
        {
            debug("Error allocating memory for request packet");
            creg_unregister(client_registry, fd);
            trans_abort(cl_trans);
            return NULL;
        }
        memset(req_pkt, 0, sizeof(XACTO_PACKET));
        void **datap = malloc(sizeof(void *));
        if (datap == NULL)
        {
            debug("Error allocating memory for datap");
            free(req_pkt);
            creg_unregister(client_registry, fd);
            trans_abort(cl_trans);
            return NULL;
        }
        debug("Waiting for request packet");
        int read_success = proto_recv_packet(fd, req_pkt, datap);
        if (read_success < 0)
        {
            debug("Error reading request packet");
            free(req_pkt);
            free(datap);
            creg_unregister(client_registry, fd);
            trans_abort(cl_trans);
            return NULL;
        }
        debug("Received request packet");
        // convert to host byte order
        req_pkt->serial = ntohl(req_pkt->serial);
        req_pkt->size = ntohl(req_pkt->size);
        req_pkt->timestamp_sec = ntohl(req_pkt->timestamp_sec);
        req_pkt->timestamp_nsec = ntohl(req_pkt->timestamp_nsec);

        switch (req_pkt->type)
        {
            case XACTO_PUT_PKT:
                debug("Received PUT request");
                memset(req_pkt, 0, sizeof(XACTO_PACKET));
                // read the key
                read_key_success = proto_recv_packet(fd, req_pkt, datap);
                if (read_key_success < 0)
                {
                    debug("Fail to read key for PUT request");
                    free(req_pkt);
                    free(datap);
                    creg_unregister(client_registry, fd);
                    trans_abort(cl_trans);
                    return NULL;
                }
                debug("Received key of size %d", ntohl(req_pkt->size));
                // convert to host byte order
                req_pkt->serial = ntohl(req_pkt->serial);
                req_pkt->size = ntohl(req_pkt->size);
                req_pkt->timestamp_sec = ntohl(req_pkt->timestamp_sec);
                req_pkt->timestamp_nsec = ntohl(req_pkt->timestamp_nsec);
                // make a key
                BLOB *tmp_blob = blob_create(*datap, req_pkt->size);
                KEY *key = key_create(tmp_blob);
                // free the datap
                free(*datap);
                // read the value
                memset(req_pkt, 0, sizeof(XACTO_PACKET));
                read_value_success = proto_recv_packet(fd, req_pkt, datap);
                if (read_value_success < 0)
                {
                    debug("Fail to read value for PUT request");
                    key_dispose(key);
                    blob_unref(tmp_blob, "packet reading for PUT failed");
                    free(req_pkt);
                    free(datap);
                    creg_unregister(client_registry, fd);
                    trans_abort(cl_trans);
                    return NULL;
                }
                debug("Received value of size %d", ntohl(req_pkt->size));
                // convert to host byte order
                req_pkt->serial = ntohl(req_pkt->serial);
                req_pkt->size = ntohl(req_pkt->size);
                req_pkt->timestamp_sec = ntohl(req_pkt->timestamp_sec);
                req_pkt->timestamp_nsec = ntohl(req_pkt->timestamp_nsec);
                // make a blob
                BLOB *value = blob_create(*datap, req_pkt->size);
                // put the key and value into the store
                TRANS_STATUS put_in_store = store_put(cl_trans, key, value);
                if (put_in_store == TRANS_ABORTED)
                {
                    debug("Transaction aborted while putting key and value into store");
                    key_dispose(key);
                    blob_unref(tmp_blob, "putting key and value into store failed");
                    blob_unref(value, "putting key and value into store failed");
                    free(req_pkt);
                    free(*datap);
                    free(datap);
                    creg_unregister(client_registry, fd);
                    return NULL;
                }
                // blob_unref(value, value->prefix);
                // send reply packet
                send_reply_packet(fd, req_pkt, put_in_store);
                // free memory
                // key_dispose(key); // THIS LINE IS CAUSING SEGFAULT BUT WHY?
                debug("unrefing tmp_blob in PUT");
                blob_unref(value, "free value in PUT");
                debug("unrefing value in PUT");
                blob_unref(tmp_blob, "free the tmp_blob in PUT");
                free(req_pkt);
                free(*datap);
                free(datap);
                break;
            case XACTO_GET_PKT:
                debug("Received GET request");
                memset(req_pkt, 0, sizeof(XACTO_PACKET));
                // read the value
                read_value_success = proto_recv_packet(fd, req_pkt, datap);
                if (read_value_success < 0)
                {
                    debug("Fail to read value for GET request");
                    free(req_pkt);
                    free(datap);
                    creg_unregister(client_registry, fd);
                    trans_abort(cl_trans);
                    return NULL;
                }
                debug("Received value of size %d", ntohl(req_pkt->size));
                // convert to host byte order
                req_pkt->serial = ntohl(req_pkt->serial);
                req_pkt->size = ntohl(req_pkt->size);
                req_pkt->timestamp_sec = ntohl(req_pkt->timestamp_sec);
                req_pkt->timestamp_nsec = ntohl(req_pkt->timestamp_nsec);
                // make a key to get the value
                BLOB *tmp_blob2 = blob_create(*datap, req_pkt->size);
                KEY *key2 = key_create(tmp_blob2);
                BLOB *value2 = NULL;
                // get the value from the store
                TRANS_STATUS get_value_stat = store_get(cl_trans, key2, &value2);
                if (get_value_stat == TRANS_ABORTED)
                {
                    debug("Transaction aborted while getting value from store");
                    // key_dispose(key2);
                    blob_unref(tmp_blob2, "getting value from store failed");
                    blob_unref(value2, "getting value from store failed");
                    free(req_pkt);
                    free(*datap);
                    free(datap);
                    creg_unregister(client_registry, fd);
                    return NULL;
                }
                // send reply packet
                send_reply_packet(fd, req_pkt, get_value_stat);
                if (value2) blob_unref(value2, "free value2 in GET");
                // send value packet
                send_value_packet(fd, req_pkt, value2 ? value2->content : NULL, value2 ? value2->size : 0, get_value_stat);
                // free memory
                blob_unref(tmp_blob2, "free tmp_blob2 in GET");
                if (value2) blob_unref(value2, "free value2 in GET");
                free(req_pkt);
                free(*datap);
                free(datap);
                break;
            case XACTO_COMMIT_PKT:
                debug("Received COMMIT request");
                // commit the transaction
                TRANS_STATUS commit_stat = trans_commit(cl_trans);
                if (commit_stat == TRANS_ABORTED)
                {
                    debug("Transaction aborted while committing");
                    free(req_pkt);
                    free(datap);
                    creg_unregister(client_registry, fd);
                    return NULL;
                }
                // send reply packet
                send_reply_packet(fd, req_pkt, commit_stat);
                creg_unregister(client_registry, fd);
                // free memory
                free(req_pkt);
                free(datap);
                // close the connection
                Close(fd);
                goto end_service;
                break;
        }
    }
    end_service:
    debug("xacto_client_service: fd %d done", fd);
    return NULL;
}