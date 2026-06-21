#ifdef BIN

#include "basic.h"
#include "perceptron.h"
#include <stdio.h>

struct {
    float input[3];
    float target[2];
} dataset[] = {
    { { 1, 2, 3 }, { 6, 7 } },
    { { 2, 2, 3 }, { 7, 6 } },
    { { 2, 2, 4 }, { 8, 8 } },
};
size_t dataset_len = ARRAY_LEN(dataset);

int main(void) {
    // 3 inputs, 2 outputs
    NEW_MAT(weights, 2, 3, {});
    NEW_MAT(biases, 2, 1, {});
    NEW_MAT(output_error, 2, 1, {});
    NEW_MAT(weight_deltas, 2, 3, {});
    Perceptron p = perceptron_new(weights, biases);
    perceptron_randomize(&p, -1.0f, 1.0f);
    for (int i = 0; i < 1000; i++) {
        for (size_t j = 0; j < dataset_len; j++) {
            MatView input = { dataset[j].input, { 3, 1 } };
            MatView target = { dataset[j].target, { 2, 1 } };
            perceptron_train(
                &p, &input, &target, 0.03f, &output_error, &weight_deltas
            );
        }
        float error_sum = 0;
        for (size_t j = 0; j < output_error.shape.n_rows; j++) {
            error_sum += output_error.data[j];
        }
        if (i % 25 == 0)
            printf("Iteration %d: Error sum = %f\n", i, error_sum);
    }
    return 0;
}

#endif
