#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include "rcsv.h"

#include "buffer_pool.h"
#include "blocking_queue.h"
#include "rthreadpool.h"

#define BUFFER_SIZE (1L << 20)
#define INIT_ALLOC  (1L << 16)

// TODO(RL) Modify this!
#define ALLOC  (1L << 32)

#define NTHREADS_POOL 4
#define BUFFER_POOL_SIZE 16

typedef struct {
    buffer_pool_t *buff_pool;
    char *buffer;
    float *array;
}  thread_arg_t;

void rcsv_thread_process(void *varg) {

    thread_arg_t *arg = (thread_arg_t*) varg;

    buffer_pool_t *buff_pool = arg->buff_pool;
    char* buffer = arg->buffer;
    float *array = arg->array;

    free(varg);

    int array_id = 0;

    char* ptr = buffer;
    for (;;) {

        char *new_ptr = NULL;
        float f = strtof(ptr, &new_ptr);
        /* float f = (float) (*ptr - '0'); */
        /* new_ptr = (f != 0 && f != 1)?ptr:ptr+1; */

        int read_something = (new_ptr - ptr) != 0;

        while (*new_ptr == ' ') {
            new_ptr++;
        }

        if (*new_ptr && *new_ptr != ',' && *new_ptr != '\n') {
            read_something = 1;
            f = NAN;

            while (*new_ptr && *new_ptr != ',' && *new_ptr != '\n') {
                new_ptr++;
            }
        }

        ptr = new_ptr + 1;

        if (read_something) {
            array[array_id++] = f;
        }

        if (*new_ptr == '\0') {
            break;
        }
    }

    buffer_pool_release(buff_pool, (void*) buffer);

}

char *search_backward(char *start, int size, char c) {
    char* ptr = start + size - 1;
    while (ptr >= start && *ptr != c) {
        ptr--;
    }
    return ptr < start ? NULL : ptr;
}

char *search_nospace_backward(char *start, int size) {
    char* ptr = start + size - 1;
    while (ptr >= start && isspace(*ptr)) {
        ptr--;
    }
    return ptr < start ? NULL : ptr;
}


int count_delim(const char *start, int size) {

    int count = 0;
    for (int i = 0; i < size; i++) {
        if (start[i] == ',' || start[i] == '\n')
            count++;
    }
    return count;
}

typedef struct {
    buffer_pool_t *buff_pool;
    blocking_queue_t *queue;
}  dispatcher_arg_t;

typedef struct {
    float *data;
    int rows;
    int cols;
}  dispatcher_return_t;

void *dispatcher_thread_func(void *varg) {

    dispatcher_arg_t *arg = (dispatcher_arg_t*) varg;

    buffer_pool_t *buff_pool = arg->buff_pool;
    blocking_queue_t *queue = arg->queue;

    dispatcher_return_t *ret = NULL;

    rthreadpool_t *pool = rthreadpool_init(NTHREADS_POOL);
    if (pool == NULL) {
        goto clean0;
    }

    float *data = (float*) malloc(sizeof(float) * ALLOC);
    if (data == NULL) {
        goto clean1;
    }


    int lines = 0;
    int elements = 0;
    for (;;) {
        char *buffer = NULL;
        int state = blocking_queue_pop(queue, (void*) &buffer);
        if (state == 1) {
            break;
        }

        int buffer_lines = 0;
        int buffer_elements = 0;

        const char* ptr = buffer;
        for (;*ptr;ptr++) {
            switch (*ptr) {
            case '\n':
                buffer_lines++;
                // No break here
            case ',':
                buffer_elements++;
                break;
            }
        }

        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->buff_pool = buff_pool;
        arg->buffer = buffer;
        arg->array = data + elements;

        int w = rthreadpool_add_work(pool, rcsv_thread_process, (void*) arg);
        if (w != 0) {
            goto clean2;
        }

        // TODO(RL) Verify that we do not need to add 1
        elements += buffer_elements + 1;
        lines += buffer_lines;

    }

    rthreadpool_join(pool);

    lines += 1;

    ret = (dispatcher_return_t *) malloc(sizeof(dispatcher_return_t));
    if (ret == NULL) {
        goto clean2;
    }

    ret->data = data;
    ret->rows = lines;
    ret->cols = elements / lines;

    goto clean1;

clean2:
    free(data);
    data = NULL;
clean1:
    rthreadpool_term(pool);
clean0:
    return ret;
}

int rcsv_read(int *rows, int *cols, float **dest, const char *path) {

    int ret = -1;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        goto clean0;
    }

    buffer_pool_t buff_pool;
    int res = buffer_pool_init(&buff_pool, BUFFER_POOL_SIZE, sizeof(char) * BUFFER_SIZE);
    if (res != 0) {
        goto clean1;
    }

    blocking_queue_t *queue = blocking_queue_init();
    if (queue == NULL) {
        goto clean2;
    }

    dispatcher_arg_t dispatcher_arg = {
        &buff_pool,
        queue
    };

    pthread_t dispatcher_thread;
    int state = pthread_create(&dispatcher_thread, NULL, dispatcher_thread_func, (void*) &dispatcher_arg);
    if (state != 0) {
        goto clean3;
    }

    char *buffer = (char*) buffer_pool_get(&buff_pool);

    size_t bytes_left = 0;
    int running = 1;
    while (running) {

        ssize_t rd = read(fd, (void*) (buffer + bytes_left), BUFFER_SIZE - bytes_left);
        if (rd < 0) {
            goto clean4;
        }

        char *split = NULL;
        char *next_buffer = NULL;
        if ((size_t) rd == BUFFER_SIZE - bytes_left) {
            split = search_backward(buffer, BUFFER_SIZE, ',');
            bytes_left = buffer + BUFFER_SIZE - split - 1;
            next_buffer = (char*) buffer_pool_get(&buff_pool);
            memcpy(next_buffer, buffer + BUFFER_SIZE - bytes_left, bytes_left);
        } else {
            split = search_nospace_backward(buffer, bytes_left + rd) + 1;
            bytes_left = 0;
            running = 0;
        }

        *split = '\0';

        blocking_queue_push(queue, buffer);

        buffer = next_buffer;

    }

    blocking_queue_push_terminate(queue);

    dispatcher_return_t *array = NULL;
    // TODO(RL) Handle pthread_join failure ?
    state = pthread_join(dispatcher_thread, (void**) &array);

    *dest = array->data;
    *rows = array->rows;
    *cols = array->cols;

    free(array);

    ret = 0;

    goto clean3;

clean4:
    pthread_cancel(dispatcher_thread);
    pthread_join(dispatcher_thread, NULL);
clean3:
    blocking_queue_term(queue);
clean2:
    buffer_pool_term(&buff_pool);
clean1:
    close(fd);
clean0:
    return ret;

}
