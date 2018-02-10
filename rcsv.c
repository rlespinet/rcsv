#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include "rcsv.h"

#define BUFFER_SIZE (1 << 16)
#define INIT_ALLOC (1 << 12)

int rcsv_read(int *rows, int *cols, float **dest, const char *path) {

    int ret = -1;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        goto clean0;
    }

    char *buffer = malloc(sizeof(char) * BUFFER_SIZE);
    if (buffer == NULL) {
        goto clean1;
    }
    buffer[BUFFER_SIZE-1] = '\0';

    int bytes_left = 0;

    char *pleft = buffer + BUFFER_SIZE;
    char *pright = NULL;

    int need_reload = 1;

    uint32_t rows_count = 0;
    uint32_t elts_count = 0;

    uint32_t data_capacity = INIT_ALLOC;
    uint32_t data_size = 0;
    float *data = (float*) malloc(sizeof(float) * data_capacity);

    int nothing_left = 0;
    for (;;) {

        if (need_reload) {

            if (nothing_left) {
                break;
            }

            if (bytes_left > 0) {
                memcpy(buffer, buffer + BUFFER_SIZE - bytes_left, bytes_left);
            }

            ssize_t rd = read(fd, (void*) (buffer + bytes_left), BUFFER_SIZE - bytes_left);
            if (rd < 0) {
                goto clean3;
            }

            if (rd < BUFFER_SIZE - bytes_left) {
                nothing_left = 1;
                buffer[bytes_left + rd] = '\0';
            } else {

                char *end_ptr = buffer + BUFFER_SIZE - 1;
                while (end_ptr >= buffer && *end_ptr != ',') {
                    end_ptr--;
                }
                if (end_ptr < buffer) {
                    goto clean3;
                }
                *end_ptr = '\0';

                bytes_left = buffer + BUFFER_SIZE - end_ptr - 1;
            }

            pleft = buffer;
            need_reload = 0;

        }
        /* float f = strtof(pleft, &pright); */
        float f = (float) (*pleft - '0');
        pright = (f != 0 && f != 1)?pleft:pleft+1;

        int valid_f = (pright - pleft > 0);

        int invalid_chars = 0;
        for (;;) {
            if (*pright == '\0') {
                need_reload = 1;
                break;
            } else if (*pright == '\n') {
                rows_count++;
                /* printf("%d\n", rows_count); */
                break;
            } else if (*pright == ',') {
                elts_count++;
                break;
            } else if (!isspace(*pright)) {
                f = NAN;
                valid_f = 1;
            }

            pright++;
        }

        if (valid_f) {

            if (data_size + 1 > data_capacity) {

                uint32_t new_data_capacity = data_capacity * 2;
                float *new_data = (float*) malloc(sizeof(float) * new_data_capacity);
                if (new_data == NULL) {
                    goto clean3;
                }

                memcpy(new_data, data, data_capacity * sizeof(float));
                free(data);

                data = new_data;
                data_capacity = new_data_capacity;
            }

            data[data_size++] = f;
        }

        pleft = pright + 1;

    }

    if (data_size % rows_count != 0) {
        // Bad read
        goto clean3;
    }

    *rows = rows_count;
    *cols = data_size / rows_count;

    *dest = data;
    ret = 0;
    goto clean2;

clean3:
    free(data);
clean2:
    free(buffer);
clean1:
    close(fd);
clean0:
    return ret;

}
