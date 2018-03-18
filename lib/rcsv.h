#pragma once

#define SUCCESS            0x00
#define ERR_OUT_OF_MEMORY  0x01
#define ERR_INTERNAL_ERROR 0x02
#define ERR_PTHREAD_ERROR  0x03
#define ERR_FILE_ERROR     0x04


int rcsv_read(int *rows, int *cols, float **dest, const char *path);
