#include "sequential_layer.h"
#include "assert.h"
#include "slice.h"

/*
 * To train a sequential layer, we need to store the intermediate outputs of
 * each layer. In this file, we call these the hidden activations, or just, the
 * activations. The activations are stored in a buffer, along with their shapes.
 *
 * The buffer consists of a header, followed by the shapes of the hidden
 * activations, then the hidden activations themselves, then the hidden
 * activation errors.
 */

typedef SequentialLayer This;

typedef struct BufHead {
    // Yeah this ended up being a pretty funny struct
    size_t n_activations;
} BufHead;

static MatShape* buf_shapes(void* buf) {
    BufHead* buf_head = (BufHead*)buf;
    static_assert(alignof(BufHead) >= alignof(MatShape));
    return (MatShape*)(buf_head + 1);
}

static float* buf_forward_data(void* buf, size_t n_layers) {
    return (float*)(buf_shapes(buf) + n_layers - 1);
}

static float* buf_backward_data(void* buf, size_t n_layers) {
    BufHead* buf_head = (BufHead*)buf;
    size_t n_activations = buf_head->n_activations;
    return (float*)(buf_shapes(buf) + n_layers - 1) + n_activations;
}

DECLARE_SLICE(const MLModel, ConstMLModelSlice);

SequentialLayer sequential_layer_new(
    VoidPtrConstSlice layer_pointers, ConstMLModelSlice layer_funcs
) {
    EXPECT(layer_pointers.len == layer_funcs.len, "Lengths should match");
    return (SequentialLayer
    ) { layer_pointers.ptr, layer_funcs.ptr, layer_funcs.len };
}

static inline size_t saturating_sub(size_t a, size_t b) {
    return a > b ? a - b : 0;
}

/// Returns the number of hidden activations
static inline size_t count_activations(const This* s, MatShape inp_shape) {
    size_t n_layers_sub_1 = saturating_sub(s->n_layers, 1);
    size_t n_activations = 0;
    MatShape curr_inp_shape = inp_shape;
    for (size_t i = 0; i < n_layers_sub_1; i++) {
        const MLModel* fs = &s->layer_funcs[i];
        void* layer = s->layer_pointers[i];
        MatShape curr_out_shape = fs->out_shape(layer, curr_inp_shape);
        n_activations += mat_shape_n_elements(curr_out_shape);
        curr_inp_shape = curr_out_shape;
    }
    return n_activations;
}

size_t sequential_layer_buf_size(const SequentialLayer* s, MatShape inp_shape) {
    size_t n_layers_sub_1 = saturating_sub(s->n_layers, 1);
    size_t n_activations = count_activations(s, inp_shape);
    static_assert(alignof(BufHead) >= alignof(MatShape));
    static_assert(alignof(MatShape) >= alignof(float));
    return sizeof(BufHead) + n_layers_sub_1 * sizeof(MatShape) +
           n_activations * sizeof(float) * 2;
}

#define DESTRUCT_BUF                                                           \
    MatShape* shapes = buf_shapes(buf);                                        \
    float* forward_data = buf_forward_data(buf, this->n_layers);               \
    float* backward_data = buf_backward_data(buf, this->n_layers);             \
    UNUSED(backward_data);

void sequential_layer_forward_with_buf(
    const This* this, const MatView* inp, MatView* out, void* buf
) {
    ((BufHead*)buf)->n_activations = count_activations(this, inp->shape);
    DESTRUCT_BUF;
    // The index in the forward data buffer where the next layer's output will
    // be written to.
    size_t data_index = 0;
    // The input matrix to the next layer.
    MatView curr_inp = *inp;
    for (size_t i = 0; i < this->n_layers; i++) {
        // The current layer,
        const MLModel* fs = &this->layer_funcs[i];
        void* layer = this->layer_pointers[i];
        // Get it's output.
        MatShape curr_out_shape = fs->out_shape(layer, curr_inp.shape);
        MatView curr_out =
            i == this->n_layers - 1
                ? *out
                : (MatView) { forward_data + data_index, curr_out_shape };
        // Save it in the buffer.
        if (i < this->n_layers - 1) shapes[i] = curr_out_shape;
        // Write
        fs->forward(layer, &curr_inp, &curr_out);
        // Ready for next iteration.
        curr_inp = curr_out;
        data_index += mat_shape_n_elements(curr_out_shape);
    }
}

void sequential_layer_forward(
    const This* this, const MatView* inp, MatView* out
) {
    // Allocate a buffer and call the implementation
    size_t buf_size = sequential_layer_buf_size(this, inp->shape);
    void* buf = malloc(buf_size);
    EXPECT_ERRNO(buf != NULL);
    sequential_layer_forward_with_buf(this, inp, out, buf);
    free(buf);
}

void sequential_layer_backward_with_buf(
    const This* this,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err,
    void* buf
) {
    DESTRUCT_BUF;
    size_t n_activations = ((BufHead*)buf)->n_activations;
    size_t n_layers = this->n_layers;
    // Start at an index just after the end of the data buffer.
    size_t data_index = n_activations;
    for (size_t i = 0; i < n_layers; i++) {
        // We are going backwards through the layers,
        size_t layer_index = n_layers - 1 - i;
        const MLModel* fs = &this->layer_funcs[layer_index];
        void* layer = this->layer_pointers[layer_index];
        // The current output shape is given from the shape array except for the
        // first iteration, where the current output shape is given by the
        // output error
        MatShape curr_out_shape =
            i == 0 ? out_err->shape : shapes[layer_index + 1];
        // The current input shape is given from the shape array except for the
        // last iteration, where the shape is given by the input
        MatShape curr_inp_shape =
            i == n_layers - 1 ? inp->shape : shapes[layer_index];
        size_t prev_data_index = data_index;
        data_index -= mat_shape_n_elements(curr_inp_shape);
        MatView curr_inp =
            i == n_layers - 1
                ? *inp
                : (MatView) { forward_data + data_index, curr_inp_shape };
        MatView curr_out_err =
            i == 0
                ? *out_err
                : (MatView) { backward_data + prev_data_index, curr_out_shape };
        MatView curr_inp_err =
            i == n_layers - 1
                ? *inp_err
                : (MatView) { backward_data + data_index, curr_inp_shape };
        fs->backward(layer, &curr_inp, &curr_out_err, &curr_inp_err);
    }
}

void sequential_layer_backward(
    const This* this,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err
) {
    // Allocate a buffer and call the implementation
    size_t buf_size = sequential_layer_buf_size(this, inp->shape);
    // We need some more space...
    size_t out_size = mat_n_elements(out_err) * sizeof(float);
    void* buf = malloc(buf_size + out_size);
    EXPECT_ERRNO(buf != NULL);
    static_assert(alignof(float) <= alignof(MatView));
    MatView out = { (float*)buf + buf_size / sizeof(float), out_err->shape };
    // The buffer must be filled with the forward pass first
    sequential_layer_forward_with_buf(this, inp, &out, buf);
    sequential_layer_backward_with_buf(this, inp, out_err, inp_err, buf);
    free(buf);
}

void sequential_layer_train(
    This* this, const MatView* inp, const MatView* out_err, float lr
) {
    // Copy pasted directly from above...
    size_t buf_size = sequential_layer_buf_size(this, inp->shape);
    size_t out_size = mat_n_elements(out_err) * sizeof(float);
    size_t inp_err_size = mat_n_elements(inp) * sizeof(float);
    void* buf = malloc(buf_size + out_size + inp_err_size);
    EXPECT_ERRNO(buf != NULL);
    static_assert(alignof(float) <= alignof(MatView));
    MatView out = { (float*)buf + buf_size / sizeof(float), out_err->shape };
    MatView inp_err = { (float*)buf + (buf_size + inp_err_size) / sizeof(float),
                        inp->shape };
    sequential_layer_forward_with_buf(this, inp, &out, buf);
    sequential_layer_backward_with_buf(this, inp, out_err, &inp_err, buf);
    //
    DESTRUCT_BUF;
    size_t n_activations = ((BufHead*)buf)->n_activations;
    size_t n_layers = this->n_layers;
    size_t data_index = n_activations;
    for (size_t i = 0; i < n_layers; i++) {
        // We are going backwards through the layers,
        size_t layer_index = n_layers - 1 - i;
        const MLModel* fs = &this->layer_funcs[layer_index];
        void* layer = this->layer_pointers[layer_index];
        // The current output shape is given from the shape array except for the
        // first iteration, where the current output shape is given by the
        // output error
        MatShape curr_out_shape =
            i == 0 ? out_err->shape : shapes[layer_index + 1];
        // The current input shape is given from the shape array except for the
        // last iteration, where the shape is given by the input
        MatShape curr_inp_shape =
            i == n_layers - 1 ? inp->shape : shapes[layer_index];
        size_t prev_data_index = data_index;
        data_index -= mat_shape_n_elements(curr_inp_shape);
        MatView curr_inp =
            i == n_layers - 1
                ? *inp
                : (MatView) { forward_data + data_index, curr_inp_shape };
        MatView curr_out_err =
            i == 0
                ? *out_err
                : (MatView) { backward_data + prev_data_index, curr_out_shape };
        fs->train(layer, &curr_inp, &curr_out_err, lr);
    }
}

bool sequential_layer_supports_inp_shape(const This* this, MatShape inp_shape) {
    MatShape shape = inp_shape;
    for (size_t i = 0; i < this->n_layers; i++) {
        const MLModel* fs = &this->layer_funcs[i];
        void* layer = this->layer_pointers[i];
        if (!fs->supports_inp_shape(layer, shape)) return false;
        shape = fs->out_shape(layer, shape);
    }
    return true;
}

MatShape sequential_layer_out_shape(const This* this, MatShape inp_shape) {
    MatShape shape = inp_shape;
    for (size_t i = 0; i < this->n_layers; i++) {
        const MLModel* fs = &this->layer_funcs[i];
        void* layer = this->layer_pointers[i];
        shape = fs->out_shape(layer, shape);
    }
    return shape;
}

MLModel sequential_layer_ml_model() {
    return (MLModel
    ) { (void (*)(const void*, const MatView*, MatView*)
        )sequential_layer_forward,
        (void (*)(const void*, const MatView*, const MatView*, MatView*)
        )sequential_layer_backward,
        (void (*)(void*, const MatView*, const MatView*, float)
        )sequential_layer_train,
        (bool (*)(const void*, MatShape))sequential_layer_supports_inp_shape,
        (MatShape(*)(const void*, MatShape))sequential_layer_out_shape };
}
