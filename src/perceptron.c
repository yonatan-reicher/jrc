#include "perceptron.h"
#include <assert.h>
#include <stdlib.h>

Perceptron perceptron_new(MatView weights, MatView bias) {
    assert(weights.shape.n_rows == bias.shape.n_rows);
    assert(bias.shape.n_cols == 1);
    return (Perceptron) { .weights = weights, .bias = bias };
}

static float rand_f_in_range(float min, float max) {
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

void perceptron_randomize(Perceptron* p, float min, float max) {
    MatShape shape = p->weights.shape;
    size_t n_weights = shape.n_rows * shape.n_cols;
    for (size_t i = 0; i < n_weights; i++) {
        p->weights.data[i] = rand_f_in_range(min, max);
    }
    for (size_t i = 0; i < p->bias.shape.n_rows; i++) {
        p->bias.data[i] = rand_f_in_range(min, max);
    }
}

void perceptron_forward(
    const Perceptron* p, const MatView* input, MatView* output
) {
    mat_mul(&p->weights, input, output);
    mat_add(output, &p->bias, output);
}

void perceptron_train(
    Perceptron* p,
    const MatView* input,
    const MatView* target,
    float lr,
    MatView* output_error,
    MatView* weight_update
) {
    // Calculate the error
    // Error = target - output
    perceptron_forward(p, input, output_error);
    mat_sub(target, output_error, output_error);
    // Update the weights!
    // W[ij] += lr * error[i] * input[j]
    mat_mul_transposed(output_error, input, weight_update);
    mat_scale(weight_update, lr, weight_update);
    mat_add(&p->weights, weight_update, &p->weights);
    // Now biases
    // b[i] += lr * error[i]
    // and we already scaled error by the learning rate, so just add
    mat_add(&p->bias, output_error, &p->bias);
}
