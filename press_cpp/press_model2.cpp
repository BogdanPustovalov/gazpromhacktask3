#include <string.h>
void score(double * input, double * output) {
    double var0[2];
    if (input[1] <= 339.7364196777344) {
        memcpy(var0, (double[]){0.9990754057941237, 0.0009245942058763099}, 2 * sizeof(double));
    } else {
        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
    }
    memcpy(output, var0, 2 * sizeof(double));
}
