#include <stdio.h>
#include <stdlib.h>
#include "rcsv.h"

int main(int argc, char **argv) {

    int rows;
    int cols;
    float *data;

    rcsv_read(&rows, &cols, &data, "data/full.csv");

    printf("%d, %d\n", rows, cols);

    free(data);

    return 0;
}
