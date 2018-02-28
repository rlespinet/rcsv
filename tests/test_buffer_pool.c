#include <stdio.h>
#include <stdlib.h>
#include "buffer_pool.h"


#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

buffer_pool_t bp;

void *thread(void *v_arg) {

    void *buffer = buffer_pool_get(&bp);

    float *fbuffer = (float*) buffer;

    for (int i = 0; i < 32; i++) {
        fbuffer[i] += 1;
    }

    sleep(1);

    buffer_pool_release(&bp, buffer);

    return NULL;

}


int main(int argc, char **argv) {

    buffer_pool_init(&bp, 8, 32 * sizeof(float));

    void* buffer[8];
    for (int i = 0; i < 8; i++) {
        buffer[i] = buffer_pool_get(&bp);
        memset(buffer[i], 0, 32 * sizeof(float));
    }

    for (int i = 0; i < 8; i++) {
        buffer_pool_release(&bp, buffer[i]);
    }

#define NTHREADS 100
    pthread_t th[NTHREADS];
    for (int i = 0; i < NTHREADS; i++) {
        pthread_create(&th[i], NULL, thread, NULL);
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(th[i], NULL);
    }

    for (int i = 0; i < 8; i++) {
        buffer[i] = buffer_pool_get(&bp);
        for (int j = 0; j < 32; j++) {
            printf("%d ", (int) ((float*)buffer[i])[j]);
        }
        printf("\n");
    }


    buffer_pool_term(&bp);

    return 0;

}
