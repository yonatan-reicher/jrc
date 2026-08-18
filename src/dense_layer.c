#include "dense_layer.h"
#include <assert.h>
#include <stdlib.h>

DenseLayer dense_layer_new(MatView weights, MatView bias) {
    assert(weights.shape.n_rows == bias.shape.n_rows);
    assert(bias.shape.n_cols == 1);
    return (DenseLayer) { weights, bias };
}

static float rand_f_in_range(float min, float max) {
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

void dense_layer_randomize(DenseLayer* p, float min, float max) {
    MatShape shape = p->weights.shape;
    size_t n_weights = shape.n_rows * shape.n_cols;
    for (size_t i = 0; i < n_weights; i++) {
        p->weights.data[i] = rand_f_in_range(min, max);
    }
    for (size_t i = 0; i < p->biases.shape.n_rows; i++) {
        p->biases.data[i] = rand_f_in_range(min, max);
    }
}

void dense_layer_forward(
    const DenseLayer* p, const MatView* inp, MatView* out
) {
    mat_mul(&p->weights, inp, out);
    mat_add(out, &p->biases, out);
}

void dense_layer_backward(
    const DenseLayer* p,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err
) {
    // Calculate the input error
    // Δinp[i] = Σ Δout[j] * w[j,i]
    // Adjusting for batching, it's actually
    // Δinp[i,j] = Σ Δout[k,j] * w[k,i]
    if (inp_err != NULL) {
        for (size_t i = 0; i < inp->shape.n_rows; i++) {
            for (size_t j = 0; j < inp->shape.n_cols; j++) {
                float sum = 0;
                for (size_t k = 0; k < out_err->shape.n_rows; k++) {
                    sum += *mat_const_get(out_err, k, j) *
                           *mat_const_get(&p->weights, k, i);
                }
                *mat_get(inp_err, i, j) = sum;
            }
        }
    }
}

void dense_layer_train(
    DenseLayer* p, const MatView* inp, const MatView* out_err, float lr
) {
    size_t n_batches = inp->shape.n_cols;
    // Update the weights!
    // w[i,j] += lr * Δout[i] * inp[j]
    // With batching,
    // w[i,j] += lr * Σ Δout[i,k] * inp[j,k]
    for (size_t i = 0; i < p->weights.shape.n_rows; i++) {
        for (size_t j = 0; j < p->weights.shape.n_cols; j++) {
            float sum = 0;
            for (size_t k = 0; k < n_batches; k++) {
                sum +=
                    *mat_const_get(out_err, i, k) * *mat_const_get(inp, j, k);
            }
            *mat_get(&p->weights, i, j) += lr * sum;
        }
    }
    // Now biases
    for (size_t i = 0; i < out_err->shape.n_rows; i++) {
        float sum = 0;
        for (size_t i_batch = 0; i_batch < n_batches; i_batch++) {
            sum += *mat_const_get(out_err, i, i_batch);
        }
        *mat_get(&p->biases, i, 0) += lr * sum;
    }
}

bool dense_layer_supports_inp_shape(
    const DenseLayer* this, MatShape inp_shape
) {
    return this->weights.shape.n_cols == inp_shape.n_rows;
}

MatShape dense_layer_out_shape(const DenseLayer* this, MatShape inp_shape) {
    assert(dense_layer_supports_inp_shape(this, inp_shape));
    return (MatShape) { this->weights.shape.n_rows, inp_shape.n_cols };
}

MLModel dense_layer_ml_model() {
    return (MLModel) {
        (void (*)(const void*, const MatView*, MatView*))dense_layer_forward,
        (void (*)(const void*, const MatView*, const MatView*, MatView*)
        )dense_layer_backward,
        (void (*)(void*, const MatView*, const MatView*, float)
        )dense_layer_train,
        (bool (*)(const void*, MatShape))dense_layer_supports_inp_shape,
        (MatShape(*)(const void*, MatShape))dense_layer_out_shape,
    };
}
