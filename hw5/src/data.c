#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "data.h"
#include "store.h"
#include "debug.h"

BLOB *blob_create(char* content, size_t size)
{
    debug("blob_create: %s", content);
    debug("size of blob: %lu", size);
    BLOB *blob = malloc(sizeof(BLOB));
    if (blob == NULL) return NULL;
    blob->content = malloc(size + 1);
    if (blob->content == NULL)
    {
        free(blob);
        return NULL;
    }
    memcpy(blob->content, content, size);
    blob->content[size] = '\0';
    blob->prefix = malloc(size + 1);
    if (blob->prefix == NULL)
    {
        free(blob->content);
        free(blob);
        return NULL;
    }
    memcpy(blob->prefix, content, size);
    blob->prefix[size] = '\0';
    blob->size = size;
    blob->refcnt = 1;
    warn("blob->content: %s", blob->content);
    pthread_mutex_init(&blob->mutex, NULL);
    return blob;
}

BLOB *blob_ref(BLOB *blob, char* why)
{
    pthread_mutex_lock(&blob->mutex);
    blob->refcnt++;
    if (strlen(why) > strlen(blob->prefix))
    {
        free(blob->prefix);
        blob->prefix = malloc(strlen(why) + 1);
    }
    memcpy(blob->prefix, why, strlen(why));
    blob->prefix[strlen(why)] = '\0';
    pthread_mutex_unlock(&blob->mutex);
    return blob;
}

void blob_unref(BLOB *blob, char* why)
{
    // info("GOT IN BLOB_UNREF");
    pthread_mutex_lock(&blob->mutex);
    info("blob_unref: %s", blob->content);
    blob->refcnt--;
    blob->prefix = why;
    if (blob->refcnt == 0)
    {
        warn("freeing blob");
        free(blob->content);
        free(blob->prefix);
        free(blob);
        pthread_mutex_unlock(&blob->mutex);
        pthread_mutex_destroy(&blob->mutex);
        return;
    }
    pthread_mutex_unlock(&blob->mutex);
}

int blob_compare(BLOB *blob1, BLOB *blob2)
{
    if (blob1->size != blob2->size) return blob1->size - blob2->size;

    return memcmp(blob1->content, blob2->content, blob1->size);
}

int blob_hash(BLOB *bp)
{
    long hash = 0;
    char* s = bp->content;
    while(*s != '\0')
    {
        hash = hash * 31 + *s;
        s++;
    }
    return hash;
}

KEY *key_create(BLOB *bp)
{
    debug("key_create: %s", bp->content);
    KEY *key = malloc(sizeof(KEY));
    debug("key: %p", key);
    if (key == NULL) return NULL;
    key->blob = blob_ref(bp, bp->prefix);
    key->hash = blob_hash(bp);
    debug("key->hash: %d", key->hash);
    debug("key->blob: %p", key->blob);
    return key;
}

void key_dispose(KEY *kp)
{
    blob_unref(kp->blob, kp->blob->prefix);
    free(kp);
}

int key_compare(KEY *kp1, KEY *kp2)
{
    if (kp1->hash != kp2->hash) return kp1->hash - kp2->hash;
    return blob_compare(kp1->blob, kp2->blob);
}

VERSION *version_create(TRANSACTION *tp, BLOB *bp)
{
    debug("version_create: %s", bp->content);
    VERSION *vp = malloc(sizeof(VERSION));
    if (vp == NULL) return NULL;
    vp->blob = blob_ref(bp, bp->prefix);
    vp->creator = tp;
    return vp;
}

void version_dispose(VERSION *vp)
{
    blob_unref(vp->blob, vp->blob->prefix);
    free(vp);
}