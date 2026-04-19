#include <string.h>
void score(double * input, double * output) {
    double var0[2];
    if (input[1] <= 505.0231018066406) {
        if (input[1] <= 27.80297565460205) {
            if (input[1] <= 4.504267454147339) {
                memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
            } else {
                if (input[2] <= 635.9833068847656) {
                    if (input[0] <= 157.94971466064453) {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){0.9861111111111112, 0.013888888888888888}, 2 * sizeof(double));
                    }
                } else {
                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                }
            }
        } else {
            if (input[2] <= 269.54844665527344) {
                if (input[0] <= 166.61954498291016) {
                    if (input[3] <= 113.54481887817383) {
                        memcpy(var0, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    }
                } else {
                    if (input[3] <= 619.0210571289062) {
                        memcpy(var0, (double[]){0.9648753073410608, 0.035124692658939236}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    }
                }
            } else {
                if (input[3] <= 305.07568359375) {
                    if (input[2] <= 740.5661315917969) {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                    }
                } else {
                    if (input[3] <= 509.5199279785156) {
                        memcpy(var0, (double[]){0.8888888888888888, 0.1111111111111111}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                    }
                }
            }
        }
    } else {
        if (input[1] <= 520.9084167480469) {
            if (input[2] <= 38.04495811462402) {
                memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
            } else {
                if (input[0] <= 851.4093322753906) {
                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                } else {
                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                }
            }
        } else {
            if (input[3] <= 27.639469623565674) {
                if (input[1] <= 691.1142272949219) {
                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                } else {
                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                }
            } else {
                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
            }
        }
    }
    memcpy(output, var0, 2 * sizeof(double));
}
