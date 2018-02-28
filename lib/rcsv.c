#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include "rcsv.h"

#include "buffer_pool.h"
#include "rthreadpool.h"

#define BUFFER_SIZE (1L << 20)
#define INIT_ALLOC  (1L << 16)

// TODO(RL) Modify this!
#define ALLOC  (1L << 32)

#define NTHREADS 4
#define BUFFER_POOL_SIZE 16

typedef struct {
    buffer_pool_t *buff_pool;
    char *buffer;
    float *array;
}  thread_arg_t;

void rcsv_thread_process(void *v_arg) {

    thread_arg_t *arg = (thread_arg_t*) v_arg;

    buffer_pool_t *buff_pool = arg->buff_pool;
    char* buffer = arg->buffer;
    float *array = arg->array;

    int array_id = 0;

    char* ptr = buffer;
    for (;;) {

        char *new_ptr = NULL;
        float f = strtof(ptr, &new_ptr);
        /* float f = (float) (*ptr - '0'); */
        /* new_ptr = (f != 0 && f != 1)?ptr:ptr+1; */

        int read_something = (new_ptr - ptr) != 0;

        while (*new_ptr && *new_ptr == ' ') {
            (*new_ptr)++;
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

int count_delim(const char *start, int size) {

    int count = 0;
    for (int i = 0; i < size; i++) {
        if (start[i] == ',' || start[i] == '\n')
            count++;
    }
    return count;
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

    rthreadpool_t *pool = rthreadpool_init(NTHREADS - 1);
    if (pool == NULL) {
        goto clean2;
    }

    size_t data_size = 0;
    float *data = (float*) malloc(sizeof(float) * ALLOC);
    if (data == NULL) {
        goto clean3;
    }

    char *buffer = (char*) buffer_pool_get(&buff_pool);
    size_t bytes_left = 0;

    int finished = 0;
    while (!finished) {

        ssize_t rd = read(fd, (void*) (buffer + bytes_left), BUFFER_SIZE - bytes_left);
        if (rd < 0) {
            goto clean4;
        }

        char *next_buffer = NULL;
        char *split = NULL;
        if (rd < BUFFER_SIZE - bytes_left) {
            split = buffer + bytes_left + rd;
            bytes_left = 0;
            finished = 1;
        } else {
            split = search_backward(buffer, BUFFER_SIZE, ',');
            bytes_left = buffer + BUFFER_SIZE - split + 1;
            next_buffer = (char*) buffer_pool_get(&buff_pool);
            memcpy(next_buffer, buffer + BUFFER_SIZE - bytes_left, bytes_left);
        }

        if (split == NULL) {
            goto clean4;
        }

        *split = '\0';

        thread_arg_t arg = {&buff_pool, buffer, data + data_size};

        int w = rthreadpool_add_work(pool, rcsv_thread_process, (void*) &arg);
        if (w != 0) {
            goto clean4;
        }

        int delim = count_delim(buffer, split - buffer);
        data_size += delim + 1;

        buffer = next_buffer;

    }

    rthreadpool_join(pool);

    *dest = data;
    *rows = 1;
    *cols = data_size;


    ret = 0;
    goto clean3;

clean4:
    free(data);
clean3:
    rthreadpool_term(pool);
clean2:
    buffer_pool_term(&buff_pool);
clean1:
    close(fd);
clean0:
    return ret;

}
