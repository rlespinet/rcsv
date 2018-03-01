#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>

#include "blocking_queue.h"

#define QUEUE_SIZE 32

struct blocking_queue {
    void *data[QUEUE_SIZE];

    int start;
    int count;

    int push_ended;
    int pop_ended;

    pthread_cond_t push_notif;
    pthread_cond_t pop_notif;
    pthread_mutex_t mtx;
};


blocking_queue_t *blocking_queue_init() {

    blocking_queue_t *queue = (blocking_queue_t*) malloc(sizeof(blocking_queue_t));
    if (queue == NULL) {
        goto err0;
    }

    queue->start = 0;
    queue->count = 0;

    queue->push_ended = 0;
    queue->pop_ended = 0;

    pthread_cond_init(&queue->push_notif, NULL);
    pthread_cond_init(&queue->pop_notif, NULL);

    pthread_mutex_init(&queue->mtx, NULL);

    return queue;

err0:
    return NULL;
}

void blocking_queue_term(blocking_queue_t *queue) {

    if (queue == NULL) {
        return;
    }

    pthread_cond_destroy(&queue->push_notif);
    pthread_cond_destroy(&queue->pop_notif);
    pthread_mutex_destroy(&queue->mtx);

    free(queue);
}

void blocking_queue_push_terminate(blocking_queue_t *queue) {

    pthread_mutex_lock(&queue->mtx);

    queue->push_ended = 1;

    pthread_cond_broadcast(&queue->pop_notif);

    pthread_mutex_unlock(&queue->mtx);

}

void blocking_queue_pop_terminate(blocking_queue_t *queue) {

    pthread_mutex_lock(&queue->mtx);

    queue->pop_ended = 1;

    pthread_cond_broadcast(&queue->push_notif);

    pthread_mutex_unlock(&queue->mtx);
}

int blocking_queue_push(blocking_queue_t *queue, void *data) {

    pthread_mutex_lock(&queue->mtx);

    while (queue->count == QUEUE_SIZE && ! queue->pop_ended) {
        pthread_cond_wait(&queue->push_notif, &queue->mtx);
    }

    int id = (queue->start + queue->count) % QUEUE_SIZE;

    queue->data[id] = data;

    queue->count++;

    pthread_cond_signal(&queue->pop_notif);

    pthread_mutex_unlock(&queue->mtx);

    return 0;

}

int blocking_queue_pop(blocking_queue_t *queue, void **data) {

    pthread_mutex_lock(&queue->mtx);

    while (!(queue->count > 0 || (queue->count == 0 && queue->push_ended))) {
        pthread_cond_wait(&queue->pop_notif, &queue->mtx);
    }

    if (queue->count == 0 && queue->push_ended) {
        pthread_mutex_unlock(&queue->mtx);
        return 1;
    }

    int id = queue->start % QUEUE_SIZE;

    *data = queue->data[id];

    queue->start = (queue->start + 1) % QUEUE_SIZE;
    queue->count--;

    pthread_cond_signal(&queue->push_notif);

    pthread_mutex_unlock(&queue->mtx);

    return 0;
}
