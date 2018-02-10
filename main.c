#include <stdio.h>
#include <stdlib.h>
#include "rcsv.h"



int main(int argc, char **argv) {

    int rows;
    int cols;
    float *data;

    rcsv_read(&rows, &cols, &data, "test_big.csv");

    /* for (int i = 0; i < cols; i++) { */
    /*     printf("%f\n", data[i]); */
    /* } */

    printf("%d, %d\n", rows, cols);
    for (int i = cols; i < 2*cols; i++) {
        printf("%f ", data[i]);
    }

    free(data);

    return 0;
}
