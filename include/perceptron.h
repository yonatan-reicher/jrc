#pragma once
#include "mat.h"

typedef struct Perceptron {
    MatView weights;
    /// This must be a column vector.
    MatView bias;
} Perceptron;

Perceptron perceptron_new(MatView weights, MatView bias);

void perceptron_randomize(Perceptron* p, float min, float max);

void perceptron_forward(
    const Perceptron* p, const MatView* input, MatView* output
);

void perceptron_train(
    Perceptron* p,
    const MatView* input,
    const MatView* target,
    float lr,
    MatView* output_error,
    MatView* weight_update
);
