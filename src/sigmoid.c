#include "sigmoid.h"
#include "basic.h"
#include <assert.h>
#include <math.h>

float sigmoid(float x) {
    return 1.f / (1.f + expf(-x));
}

float sigmoid_deriv(float x) {
    // TODO:
    const float dx = 0.001f;
    return (sigmoid(dx + x) - sigmoid(x)) / dx;
}

float sigmoid_backward(float inp, float out_err) {
    return sigmoid_deriv(inp) * out_err;
}

#define MY_MAT_MAP(OUT)                                                        \
    ASSERT(mat_shape_eq(inp->shape, (OUT)->shape), "");                            \
    size_t n_elements = mat_n_elements(inp);                                   \
    for (size_t i = 0; i < n_elements; i++)

void sigmoid_mat(const MatView* inp, MatView* out) {
    MY_MAT_MAP(out) {
        out->data[i] = sigmoid(inp->data[i]);
    }
}

void sigmoid_deriv_mat(const MatView* inp, MatView* out) {
    MY_MAT_MAP(out) {
        out->data[i] = sigmoid_deriv(inp->data[i]);
    }
}

void sigmoid_backward_mat(
    const MatView* inp, const MatView* out_err, MatView* inp_err
) {
    MY_MAT_MAP(inp_err) {
        inp_err->data[i] = sigmoid_backward(inp->data[i], out_err->data[i]);
    }
}

static void sigmoid_mat_with_self(
    const void*, const MatView* inp, MatView* out
) {
    sigmoid_mat(inp, out);
}

static void sigmoid_backward_mat_with_self(
    const void*, const MatView* inp, const MatView* out_err, MatView* inp_err
) {
    sigmoid_backward_mat(inp, out_err, inp_err);
}

static void sigmoid_train(
    void* self, const MatView* inp, const MatView* out_err, float lr
) {
    UNUSED(self);
    UNUSED(inp);
    UNUSED(out_err);
    UNUSED(lr);
}

static bool sigmoid_supports_inp_shape(const void* self, MatShape inp_shape) {
    UNUSED(self);
    UNUSED(inp_shape);
    return true;
}

static MatShape sigmoid_out_shape(const void* self, MatShape inp_shape) {
    UNUSED(self);
    return inp_shape;
}

MLModel sigmoid_ml_model() {
    return (MLModel) {
        sigmoid_mat_with_self, sigmoid_backward_mat_with_self,
        sigmoid_train,         sigmoid_supports_inp_shape,
        sigmoid_out_shape,
    };
}
