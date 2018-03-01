#pragma once

#define QUEUE_SIZE 32

typedef struct blocking_queue blocking_queue_t;

blocking_queue_t *blocking_queue_init();

void blocking_queue_term(blocking_queue_t *queue);

void blocking_queue_push_terminate(blocking_queue_t *queue);

void blocking_queue_pop_terminate(blocking_queue_t *queue);

int blocking_queue_push(blocking_queue_t *queue, void *data);

int blocking_queue_pop(blocking_queue_t *queue, void **data);
