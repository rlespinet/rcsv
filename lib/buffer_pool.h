#pragma once

#include "buffer_pool.h"

typedef struct buffer_pool buffer_pool_t;

struct buffer_pool {
    size_t capacity;
    size_t buffer_size;
    void **data;
    int* free_ids;
    int free_count;
    pthread_mutex_t mtx;
    pthread_cond_t cnd;
};

int buffer_pool_init(buffer_pool_t* bp, size_t capacity, size_t buffer_size);

void buffer_pool_term(buffer_pool_t *bp);

void *buffer_pool_get(buffer_pool_t *bp);

void buffer_pool_release(buffer_pool_t *bp, void *buffer);

#define buffer_pool_init_t(bp, type, capacity, buffer_size) \
    buffer_pool_init(bp, sizeof(type) * capacity, buffer_size)
