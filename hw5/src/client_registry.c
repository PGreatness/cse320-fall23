#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "client_registry.h"
#include "student/csapp.h"
#include "debug.h"

typedef struct client
{
    int fd;
    struct client* next;
} CLIENT;

typedef struct client_registry
{
    int total_clients;
    pthread_mutex_t mutex;
    sem_t sem;
    CLIENT* head;
} CLIENT_REGISTRY;

CLIENT_REGISTRY* creg_init()
{
    CLIENT_REGISTRY* cr = malloc(sizeof(CLIENT_REGISTRY));
    if (cr == NULL)
    {
        return NULL;
    }
    if (pthread_mutex_init(&cr->mutex, NULL) != 0)
    {
        free(cr);
        return NULL;
    }
    if (sem_init(&cr->sem, 0, 0) != 0)
    {
        pthread_mutex_destroy(&cr->mutex);
        free(cr);
        return NULL;
    }

    debug("Initialized client registry\n");
    cr->total_clients = 0;
    cr->head = NULL;
    return cr;
}

int creg_register(CLIENT_REGISTRY* cr, int fd)
{
    if (cr == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    CLIENT* client = malloc(sizeof(CLIENT));
    if (client == NULL)
    {
        errno = ENOMEM;
        return -1;
    }
    client->fd = fd;
    client->next = NULL;

    pthread_mutex_lock(&cr->mutex);
    if (cr->head == NULL)
    {
        cr->head = client;
    }
    else
    {
        CLIENT* curr = cr->head;
        while (curr->next != NULL)
        {
            if (curr->fd == fd)
            {
                // Already registered
                free(client);
                pthread_mutex_unlock(&cr->mutex);
                return -1;
            }
            curr = curr->next;
        }
        curr->next = client;
    }
    cr->total_clients++;
    debug("Registered client with fd: %d, total clients now: %i\n", fd, cr->total_clients);
    sem_post(&cr->sem);

    pthread_mutex_unlock(&cr->mutex);
    return 0;
}

int creg_unregister(CLIENT_REGISTRY* cr, int fd)
{
    if (cr == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    pthread_mutex_lock(&cr->mutex);
    CLIENT* curr = cr->head, *prev = cr->head;
    while (curr != NULL)
    {
        if (curr->fd == fd)
        {
            if (curr == cr->head)
            {
                cr->head = curr->next;
            }
            else
            {
                prev->next = curr->next;
            }
            free(curr);
            cr->total_clients--;
            debug("Unregistered client with fd: %d, total clients now: %i\n", fd, cr->total_clients);
            sem_post(&cr->sem);
            pthread_mutex_unlock(&cr->mutex);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    // Not found
    errno = EINVAL;
    pthread_mutex_unlock(&cr->mutex);
    return -1;
}

void creg_wait_for_empty(CLIENT_REGISTRY* cr)
{
    debug("Awaiting empty client registry\n");
    if (cr == NULL)
    {
        errno = EINVAL;
        return;
    }
    while(1)
    {
        sem_wait(&cr->sem);
        pthread_mutex_lock(&cr->mutex);
        debug("Lock acquired, checking if all clients unregistered\n");
        if (cr->head == NULL)
        {
            debug("All clients unregistered\n");
            pthread_mutex_unlock(&cr->mutex);
            return;
        }
        debug("Waiting for %i clients to unregister\n", cr->total_clients);
        pthread_mutex_unlock(&cr->mutex);
    }
}

void creg_fini(CLIENT_REGISTRY* cr)
{
    if (cr == NULL)
    {
        errno = EINVAL;
        return;
    }
    pthread_mutex_lock(&cr->mutex);
    CLIENT* curr = cr->head;
    if (curr != NULL)
    {
        // There are still registered clients
        debug("Error: registered clients still exist\n");
        pthread_mutex_unlock(&cr->mutex);
        return;
    }
    debug("Finalizing client registry\n");
    pthread_mutex_unlock(&cr->mutex);
    pthread_mutex_destroy(&cr->mutex);
    sem_destroy(&cr->sem);
    free(cr);
}

void creg_shutdown_all(CLIENT_REGISTRY* cr)
{
    if (cr == NULL)
    {
        errno = EINVAL;
        return;
    }
    pthread_mutex_lock(&cr->mutex);
    CLIENT* curr = cr->head;
    while (curr != NULL)
    {
        shutdown(curr->fd, SHUT_RDWR);
        curr = curr->next;
        cr->total_clients--;
    }
    sem_post(&cr->sem);
    pthread_mutex_unlock(&cr->mutex);
}
