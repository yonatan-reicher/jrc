#include "relu.h"

float relu(float x, float leak) {
    return x >= 0.0 ? x : leak * x;
}

float relu_deriv(float x, float leak) {
    return x >= 0.0 ? 1.0 : leak;
}

void relu_forward(const Relu* this, const MatView* inp, MatView* out) {
    for (size_t y = 0; y < inp->shape.n_rows; y++)
        for (size_t x = 0; x < inp->shape.n_cols; x++)
            *mat_get(out, y, x) = relu(*mat_const_get(inp, y, x), this->leak);
}

void relu_backward(
    const Relu* this,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err
) {
    for (size_t y = 0; y < inp->shape.n_rows; y++)
        for (size_t x = 0; x < inp->shape.n_cols; x++)
            *mat_get(inp_err, y, x) =
                relu_deriv(*mat_const_get(inp, y, x), this->leak) *
                *mat_const_get(out_err, y, x);
}

void relu_train(void*, const MatView*, const MatView*, float) {
}
bool relu_supports_input_shape(const void*, MatShape) {
    return true;
}
MatShape relu_out_shape(const void*, MatShape inp_shape) {
    return inp_shape;
}

MLModel relu_ml_model() {
    return (MLModel) {
        (MLModelForward*)relu_forward,
        (MLModelBackward*)relu_backward,
        (MLModelTrain*)relu_train,
        (MLModelSupportsInpShape*)relu_supports_input_shape,
        (MLModelOutShape*)relu_out_shape,
    };
}
