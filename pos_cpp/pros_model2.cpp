#include <string.h>
void score(double * input, double * output) {
    double var0[2];
    if (input[2] <= 0.06526506692171097) {
        if (input[0] <= 56.691120624542236) {
            memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
        } else {
            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
        }
    } else {
        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
    }
    memcpy(output, var0, 2 * sizeof(double));
}
