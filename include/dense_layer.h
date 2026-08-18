#pragma once
#include "mat.h"
#include "ml_model.h"

typedef struct DenseLayer {
    MatView weights;
    MatView biases;
} DenseLayer;

DenseLayer dense_layer_new(MatView weights, MatView biases);

void dense_layer_randomize(DenseLayer*, float min, float max);

void dense_layer_forward(const DenseLayer*, const MatView* inp, MatView* out);

void dense_layer_backward(
    const DenseLayer*,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err
);

void dense_layer_train(
    DenseLayer*,
    const MatView* inp,
    const MatView* out_err,
    float lr
);

bool dense_layer_supports_inp_shape(const DenseLayer*, MatShape inp_shape);

MatShape dense_layer_out_shape(const DenseLayer*, MatShape inp_shape);

MLModel dense_layer_ml_model();
