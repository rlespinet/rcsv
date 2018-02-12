#include <stdlib.h>
#include <pthread.h>

#include "buffer_pool.h"

#define BUFFER_OFFSET(ptr, offset) (((char*)ptr) + (offset))

int buffer_pool_init(buffer_pool_t* bp, size_t capacity, size_t buffer_size) {

    if (bp == NULL) {
        goto err0;
    }

    bp->capacity = capacity;
    bp->buffer_size = buffer_size;

    bp->data = (void **) malloc(capacity * sizeof(void*));
    if (bp->data == NULL) {
        goto err0;
    }

    bp->data[0] = (void*) malloc(buffer_size * capacity);
    if (bp->data[0] == NULL) {
        goto err1;
    }

    for (int i = 1; i < capacity; i++) {
        bp->data[i] = BUFFER_OFFSET(bp->data[0], i * buffer_size);
    }

    bp->free_ids = (int*) malloc(capacity * sizeof(int));
    if (bp->free_ids == NULL) {
        goto err2;
    }

    for (int i = 0; i < capacity; i++) {
        bp->free_ids[i] = i;
    }

    bp->free_count = capacity;

    pthread_mutex_init(&bp->mtx, NULL);
    pthread_cond_init(&bp->cnd, NULL);

    return 0;

err2:
    free(bp->data[0]);
err1:
    free(bp->data);
err0:
    return -1;
}

void buffer_pool_term(buffer_pool_t *bp) {

    if (bp == NULL) {
        return;
    }

    if (bp->data != NULL) {
        if (bp->data[0] != NULL) {
            free(bp->data[0]);
        }
        free(bp->data);
    }

    if (bp->free_ids != NULL) {
        free(bp->free_ids);
    }


}

buffer_t buffer_pool_get(buffer_pool_t *bp) {

    if (bp == NULL) {
        return (buffer_t){NULL, -1};
    }

    pthread_mutex_lock(&bp->mtx);

    while (bp->free_count == 0) {
        pthread_cond_wait(&bp->cnd, &bp->mtx);
    }

    (bp->free_count)--;

    int id = bp->free_ids[bp->free_count];

    buffer_t buffer = {
        bp->data[id],
        id
    };

    pthread_mutex_unlock(&bp->mtx);


    return buffer;

}

void buffer_pool_release(buffer_pool_t *bp, buffer_t buffer) {

    if (bp == NULL) {
        return;
    }

    pthread_mutex_lock(&bp->mtx);

    bp->free_ids[bp->free_count] = buffer.id;

    pthread_cond_signal(&bp->cnd);

    bp->free_count++;

    pthread_mutex_unlock(&bp->mtx);
}
