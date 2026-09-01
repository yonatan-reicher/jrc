#include <locale.h>
#include <math.h>
#ifdef BIN

#include "relu.h"
#include "array.h"
#include "basic.h"
#include "console_graphics.h"
#include "dense_layer.h"
#include "idx_file_format.h"
#include "plot.h"
#include "sequential_layer.h"
#include "sigmoid.h"
#include "slice.h"
#include "str.h"
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// struct {
//     float input[3];
//     float target[2];
// } dataset[] = {
//     { { 0, 0, 0 }, { 1, 0 } }, { { 1, 0, 0 }, { 0, 0 } },
//     { { 0, 1, 0 }, { 0, 0 } }, { { 1, 1, 0 }, { 1, 0 } },
//     { { 0, 0, 0 }, { 1, 0 } }, { { 2, 0, 0 }, { 0, 0 } },
//     { { 0, 2, 0 }, { 0, 0 } }, { { 2, 2, 0 }, { 1, 0 } },
//     { { 0, 0, 0 }, { 1, 0 } }, { { 1, 2, 0 }, { 1, 0 } },
//     { { 2, 1, 0 }, { 1, 0 } }, { { 2, 0.5f, 0 }, { 1, 0 } },
// };
// size_t dataset_len = ARRAY_LEN(dataset);

DECLARE_SLICE(const MLModel, ConstMLModelSlice);
DECLARE_SLICE(const float, ConstFloatSlice);
DECLARE_ARRAY(float, FloatArray);

typedef struct A {
    pthread_mutex_t m;
    FloatArray errors;
    size_t n;
} A;

void plot_draw(unsigned x, unsigned y, wchar_t c, void* arg) {
    struct {
        wchar_t* start;
        unsigned screen_width;
    }* a = arg;
    a->start[(size_t)x + (size_t)y * (size_t)a->screen_width] = c;
}

void draw(wchar_t* buf, uint16_t w, uint16_t h, void* arg) {
    A* a = arg;

    pthread_mutex_lock(&a->m);
    FloatArray errors = array_clone(&a->errors);
    size_t n = a->n;
    pthread_mutex_unlock(&a->m);

    // while (errors.len > (size_t)(0.7 * (double)n)) {
    //     array_pop_at(&errors, 0);
    // }

    if (errors.len > 0 && w > 0 && h > 0) {
        double max = 0.1;
        SLICE_FOREACH(&errors, e) max = MAX(max, *e);
#define M 1024
        wchar_t x_label[M], y_label[M];
        swprintf(x_label, M, L"%zu/%zu", (size_t)errors.len, n);
        swprintf(y_label, M, L"%lf", (double)max);
        struct {
            wchar_t* start;
            unsigned screen_width;
        } plot_arg = { buf + 0, w };
        plot_bar_to_func(
            plot_draw,
            &plot_arg,
            (ConstFloatSlice)slice_of_array(&errors),
            w,
            h - 14,
            x_label,
            y_label
        );
    }
    array_free(&errors);
}

static MatView one_hot(uint8_t labels[], float* a, size_t batch_size) {
    for (size_t i = 0; i < batch_size * 10; i++) a[i] = 0; // zeros
    for (size_t i = 0; i < batch_size; i++) {
        uint8_t label = labels[i];
        EXPECT(0 <= label && label <= 9, "out of range");
        a[label * 10 + i] = 1;
    }
    return (MatView) { a, { 10, batch_size } };
}

int main(void) {
    EXPECT(setlocale(LC_ALL, ""), "failed to set locale");
    srand((unsigned int)time(NULL));

    // Initialize ncurses.
    initscr();
    cbreak();
    noecho();

#define N 12000

    A a = { {}, array_empty(), N };
    pthread_mutex_init(&a.m, NULL);

    ConsoleGraphics con;
    con = console_graphics_init(stdout, draw, &a, 60, false);

    // load input
    FloatArray image_data = array_empty();
    FILE* f = fopen("/Users/jr/Downloads/train-images-idx3-ubyte", "rbe");
    IdxReadErr e = idx_file_read_mnist_images(f, &image_data);
    fclose(f);
    if (e) {
        fprintf(stderr, "%s", idx_read_err_to_str(e));
        exit(1);
    }
    f = fopen("/Users/jr/Downloads/train-labels-idx1-ubyte", "rbe");
    IdxReadResult r = idx_file_read(f);
    fclose(f);
    if (r.err) {
        fprintf(stderr, "%s", idx_read_err_to_str(e));
        exit(1);
    }
    UInt8Array labels = r.data;
    size_t image_size = 28 * 28;
    size_t n_images = image_data.len / image_size;

    // image_size inputs, 10 outputs
    size_t hidden = 100;
    size_t batch_size = 5;
    NEW_MAT(weights1, hidden, image_size, {});
    NEW_MAT(biases1, hidden, 1, {});
    NEW_MAT(weights2, hidden, hidden, {});
    NEW_MAT(biases2, hidden, 1, {});
    NEW_MAT(weights3, 10, hidden, {});
    NEW_MAT(biases3, 10, 1, {});
    NEW_MAT(out, 10, batch_size, {});
    NEW_MAT(output_error, 10, batch_size, {});
    DenseLayer _p1 = dense_layer_new(weights1, biases1);
    dense_layer_randomize(&_p1, -1.0f, 1.0f);
    DenseLayer _p2 = dense_layer_new(weights2, biases2);
    dense_layer_randomize(&_p2, -1.0f, 1.0f);
    DenseLayer _p3 = dense_layer_new(weights3, biases3);
    dense_layer_randomize(&_p3, -1.0f, 1.0f);
    Relu _r1 = { 0.1f };
    Relu _r2 = { 0.1f };
    SequentialLayer l = sequential_layer_new(
        slice(VoidPtrConstSlice, { &_p1, &_r1, &_p2, &_r2, &_p3, NULL }),
        slice(
            ConstMLModelSlice,
            {
                dense_layer_ml_model(),
                relu_ml_model(),
                dense_layer_ml_model(),
                relu_ml_model(),
                dense_layer_ml_model(),
                sigmoid_ml_model(),
            }
        )
    );
    for (size_t i = 0; i < N; i++) {
        // size_t i_image = ((size_t)rand()) % n_images;
        size_t i_image =
            batch_size * ((size_t)rand() % (n_images / batch_size));
        // size_t i_image = 0;
        float* start = array_get(&image_data, i_image * image_size);
        MatView image = { start, { image_size, batch_size } };
        uint8_t* labels_start = array_get(&labels, i_image);
        float one_hot_buf[10][batch_size] = {};
        MatView target = one_hot(labels_start, &one_hot_buf[0][0], batch_size);

        mat_transpose_inplace(&image);
        sequential_layer_forward(&l, &image, &out);
        mat_sub(&target, &out, &output_error);
        float lr = 0.005f;
        sequential_layer_train(&l, &image, &output_error, lr);
        mat_transpose_inplace(&image);

        float error_sum = 0;
        for (size_t j = 0; j < output_error.shape.n_rows; j++)
            for (size_t k = 0; k < output_error.shape.n_cols; k++)
                error_sum += fabsf(*mat_const_get(&output_error, j, k));
        error_sum /= (float)batch_size;
        pthread_mutex_lock(&a.m);
        array_push(&a.errors, error_sum);
        pthread_mutex_unlock(&a.m);
    }
    // char s[1024] = "";
    int c = ' ';
    while (true) {
        c = getch();
        if (c == 'q') break;
    }
    UNUSED(con);
    return 0;
}

#endif

/// C compilers throw warnings when a source file does not export a symbol and
/// the `-pedantic` file is on.
int make_compiler_happy = 0;
