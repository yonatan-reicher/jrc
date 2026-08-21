#ifdef BIN

#include "array.h"
#include "basic.h"
#include "dense_layer.h"
#include "plot.h"
#include "sequential_layer.h"
#include "sigmoid.h"
#include "slice.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

struct {
    float input[3];
    float target[2];
} dataset[] = {
    { { 0, 0, 0 }, { 1, 0 } }, { { 1, 0, 0 }, { 0, 0 } },
    { { 0, 1, 0 }, { 0, 0 } }, { { 1, 1, 0 }, { 1, 0 } },
    { { 0, 0, 0 }, { 1, 0 } }, { { 2, 0, 0 }, { 0, 0 } },
    { { 0, 2, 0 }, { 0, 0 } }, { { 2, 2, 0 }, { 1, 0 } },
    { { 0, 0, 0 }, { 1, 0 } }, { { 1, 2, 0 }, { 1, 0 } },
    { { 2, 1, 0 }, { 1, 0 } }, { { 2, 0.5f, 0 }, { 1, 0 } },
};
size_t dataset_len = ARRAY_LEN(dataset);

DECLARE_SLICE(const MLModel, ConstMLModelSlice);
DECLARE_SLICE(const float, ConstFloatSlice);
DECLARE_ARRAY(float, FloatArray);

int main(void) {
    srand((unsigned int)time(NULL));
    // 3 inputs, 2 outputs
    NEW_MAT(weights1, 2, 3, {});
    NEW_MAT(biases1, 2, 1, {});
    NEW_MAT(weights2, 2, 2, {});
    NEW_MAT(biases2, 2, 1, {});
    NEW_MAT(out, 2, 1, {});
    NEW_MAT(output_error, 2, 1, {});
    DenseLayer _p1 = dense_layer_new(weights1, biases1);
    dense_layer_randomize(&_p1, -1.0f, 1.0f);
    DenseLayer _p2 = dense_layer_new(weights2, biases2);
    dense_layer_randomize(&_p2, -1.0f, 1.0f);
    SequentialLayer l = sequential_layer_new(
        slice(VoidPtrConstSlice, { &_p1, NULL, &_p2 }),
        slice(
            ConstMLModelSlice,
            { dense_layer_ml_model(),
              sigmoid_ml_model(),
              dense_layer_ml_model() }
        )
    );
    FloatArray errors = array_empty();
    for (int i = 0; i < 120000; i++) {
        for (size_t j = 0; j < dataset_len; j++) {
            MatView input = { dataset[j].input, { 3, 1 } };
            MatView target = { dataset[j].target, { 2, 1 } };
            sequential_layer_forward(&l, &input, &out);
            mat_sub(&target, &out, &output_error);
            sequential_layer_train(&l, &input, &output_error, 0.023f);
        }
        float error_sum = 0;
        for (size_t j = 0; j < output_error.shape.n_rows; j++) {
            error_sum += output_error.data[j];
        }
        array_push(&errors, error_sum);
        // if (i % 25 == 0) printf("Iteration %d: Error sum = %f\n", i,
        // error_sum);
        if (i % 1000 == 0) {
            double max = -10000;
            SLICE_FOREACH(&errors, e) max = MAX(max, *e);
#define M 1024
            wchar_t x_label[M], y_label[M];
            swprintf(x_label, M, L"%zu", (size_t)errors.len);
            swprintf(y_label, M, L"%lf", (double)max);
            printf("\n");
            system("clear");
            plot_bar(
                stdout,
                (ConstFloatSlice)slice_of_array(&errors),
                150,
                35,
                x_label,
                y_label
            );
            printf("Final error: %f\n", errors.ptr[errors.len - 1]);
            usleep(5000);
        }
    }
    return 0;
}

#endif

/// C compilers throw warnings when a source file does not export a symbol and
/// the `-pedantic` file is on.
int make_compiler_happy = 0;
