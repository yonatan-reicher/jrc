#pragma once
#include "mat.h"
#include "ml_model.h"

typedef struct SequentialLayer {
    void* const* layer_pointers;
    MLModel const* layer_funcs;
    size_t n_layers;
} SequentialLayer;

struct ConstMLModelSlice;
struct VoidPtrConstSlice;
SequentialLayer sequential_layer_new(
    struct VoidPtrConstSlice, struct ConstMLModelSlice
);

size_t sequential_layer_buf_size(const SequentialLayer* s, MatShape inp_shape);

/// The buffer given is used to store intermediate results of the forward pass.
/// It must be at least as large as the sum of the output sizes of all layers
/// except the last one.
void sequential_layer_forward_with_buf(
    const SequentialLayer*, const MatView* inp, MatView* out, void* buf
);
void sequential_layer_forward(
    const SequentialLayer*, const MatView* inp, MatView* out
);

/// The buffer given is used both to store the intermediate results of the
/// forward pass, and also to store the intermediate results of the backward
/// pass. It must be at least as large as the sum of the output sizes of all
/// layers except the last one.
void sequential_layer_backward_with_buf(
    const SequentialLayer*,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err,
    void* buf
);
void sequential_layer_backward(
    const SequentialLayer*,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err
);

void sequential_layer_train(
    SequentialLayer* this, const MatView* inp, const MatView* out_err, float lr
);

MLModel sequential_layer_ml_model();
