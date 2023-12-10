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
    blob->prefix = "create blob with content";
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
    debug("blob_ref: %s, refcnt: %d", blob->content, blob->refcnt);
    blob->prefix = why;
    pthread_mutex_unlock(&blob->mutex);
    return blob;
}

void blob_unref(BLOB *blob, char* why)
{
    // info("GOT IN BLOB_UNREF");
    pthread_mutex_lock(&blob->mutex);
    debug("WHY: %s", why);
    // info("blob_unref: %p", blob);
    blob->refcnt--;
    info("blob_unref: %s, refcnt: %d", blob->content, blob->refcnt);
    blob->prefix = why;
    if (blob->refcnt == 0)
    {
        warn("freeing blob");
        free(blob->content);
        pthread_mutex_unlock(&blob->mutex);
        pthread_mutex_destroy(&blob->mutex);
        free(blob);
        return;
    }
    pthread_mutex_unlock(&blob->mutex);
}

int blob_compare(BLOB *blob1, BLOB *blob2)
{
    if (blob1->size != blob2->size) return blob1->size - blob2->size;

    return blob_hash(blob1) - blob_hash(blob2);
}

int blob_hash(BLOB *bp)
{
    long hash = 0;
    int large_prime = 59; // generated from https://bigprimes.org/
    char* s = bp->content;
    while(*s != '\0')
    {
        hash = hash * large_prime + *s;
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
    key->blob = blob_ref(bp, "create key from blob");
    key->hash = blob_hash(bp);
    debug("key->hash: %d", key->hash);
    debug("key->blob: %p", key->blob);
    return key;
}

void key_dispose(KEY *kp)
{
    blob_unref(kp->blob, "dispose of key");
    free(kp);
}

int key_compare(KEY *kp1, KEY *kp2)
{
    if (kp1->hash != kp2->hash) return kp1->hash - kp2->hash;
    return blob_compare(kp1->blob, kp2->blob);
}

VERSION *version_create(TRANSACTION *tp, BLOB *bp)
{
    // debug("version_create: %s", bp->content);
    debug("version_create: %p", bp);
    VERSION *vp = malloc(sizeof(VERSION));
    if (vp == NULL) return NULL;
    memset(vp, 0, sizeof(VERSION));
    vp->blob = bp;
    vp->creator = tp;
    if (tp) trans_ref(tp, "version create");
    if (bp) blob_ref(bp, "remove this");
    return vp;
}

void version_dispose(VERSION *vp)
{
    debug("Dispose of version %p", vp);
    if (vp->creator) trans_unref(vp->creator, "version dispose of creator");
    if (vp->blob) blob_unref(vp->blob, "version dispose of blob");
    free(vp);
}