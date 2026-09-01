#pragma once
#include "mat.h"
#include "ml_model.h"

typedef struct Relu {
    float leak;
} Relu;

float relu(float x, float leak);

float relu_deriv(float x, float leak);

void relu_forward(const Relu*, const MatView* inp, MatView* out);

void relu_backward(
    const Relu*, const MatView* inp, const MatView* out_err, MatView* inp_err
);

MLModel relu_ml_model();
