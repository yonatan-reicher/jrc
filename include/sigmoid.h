#pragma once
#include "mat.h"
#include "ml_model.h"

float sigmoid(float x);
void sigmoid_mat(const MatView* inp, MatView* out);

float sigmoid_deriv(float x);
void sigmoid_deriv_mat(const MatView* inp, MatView* out);

float sigmoid_backward(float inp, float out_err);
void sigmoid_backward_mat(
    const MatView* inp, const MatView* out_err, MatView* inp_err
);

/// The sigmoid activation function
MLModel sigmoid_ml_model();
