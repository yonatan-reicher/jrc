#include "mat.h"
#include "basic.h"
#include <assert.h>
#include <math.h>
#include <string.h>

// ------ Matrix Shape ---------------------------------------------------------

MatShape mat_shape_zero(void) {
    MatShape shape = { 0, 0 };
    return shape;
}

bool mat_shape_is_zero(MatShape shape) {
    return shape.n_rows == 0 || shape.n_cols == 0;
}

bool mat_shape_try_mul(MatShape lhs, MatShape rhs, MatShape* out) {
    if (lhs.n_cols != rhs.n_rows) return false;
    out->n_rows = lhs.n_rows;
    out->n_cols = rhs.n_cols;
    return true;
}

MatShape mat_shape_mul(MatShape lhs, MatShape rhs) {
    MatShape out;
    bool success = mat_shape_try_mul(lhs, rhs, &out);
#ifndef NDEBUG
    EXPECT(
        success,
        "bad multiplication of matrices of sizes %zu x %zu and %zu x %zu",
        (unsigned long)lhs.n_rows,
        (unsigned long)lhs.n_cols,
        (unsigned long)rhs.n_rows,
        (unsigned long)rhs.n_cols
    );
#else
    UNUSED(success);
#endif
    return out;
}

bool mat_shape_eq(MatShape a, MatShape b) {
    return a.n_rows == b.n_rows && a.n_cols == b.n_cols;
}

MatShape mat_shape_transposed(MatShape shape) {
    return (MatShape) { shape.n_cols, shape.n_rows };
}

// ------ Matrix Operations ----------------------------------------------------

void mat_mul(const MatView* a, const MatView* b, MatView* c) {
    assert(a->shape.n_cols == b->shape.n_rows);
    c->shape = mat_shape_mul(a->shape, b->shape);
    for (size_t i_row = 0; i_row < a->shape.n_rows; i_row++) {
        for (size_t i_col = 0; i_col < b->shape.n_cols; i_col++) {
            float sum = 0;
            for (size_t i = 0; i < a->shape.n_cols; i++) {
                sum += a->data[i_row * a->shape.n_cols + i] *
                       b->data[i * b->shape.n_cols + i_col];
            }
            c->data[i_row * c->shape.n_cols + i_col] = sum;
        }
    }
}

void mat_mul_transposed(const MatView* a, const MatView* b, MatView* c) {
    assert(a->shape.n_cols == b->shape.n_cols);
    c->shape = mat_shape_mul(a->shape, mat_shape_transposed(b->shape));
    for (size_t i_row = 0; i_row < a->shape.n_rows; i_row++) {
        for (size_t i_col = 0; i_col < b->shape.n_rows; i_col++) {
            float sum = 0;
            for (size_t i = 0; i < a->shape.n_cols; i++) {
                sum += a->data[i_row * a->shape.n_cols + i] *
                       b->data[i_col * b->shape.n_cols + i];
            }
            c->data[i_row * c->shape.n_cols + i_col] = sum;
        }
    }
}

void mat_add(const MatView* a, const MatView* b, MatView* c) {
    assert(mat_shape_eq(a->shape, b->shape));
    c->shape = a->shape;
    size_t n_elements = a->shape.n_rows * a->shape.n_cols;
    for (size_t i = 0; i < n_elements; i++) {
        c->data[i] = a->data[i] + b->data[i];
    }
}

void mat_sub(const MatView* a, const MatView* b, MatView* c) {
    assert(mat_shape_eq(a->shape, b->shape));
    c->shape = a->shape;
    size_t n_elements = a->shape.n_rows * a->shape.n_cols;
    for (size_t i = 0; i < n_elements; i++) {
        c->data[i] = a->data[i] - b->data[i];
    }
}

void mat_scale(const MatView* mat, float scalar, MatView* out) {
    out->shape = mat->shape;
    size_t n_elements = mat->shape.n_rows * mat->shape.n_cols;
    for (size_t i = 0; i < n_elements; i++) {
        out->data[i] = mat->data[i] * scalar;
    }
}

bool mat_eq(const MatView* A, const MatView* B, float epsilon) {
    if (!mat_shape_eq(A->shape, B->shape)) return false;
    assert(epsilon >= 0);
    size_t n_elements = A->shape.n_rows * A->shape.n_cols;
    for (size_t i = 0; i < n_elements; i++) {
        float diff = A->data[i] - B->data[i];
        if (fabsf(diff) > epsilon) return false;
    }
    return true;
}

// ------ Helpers --------------------------------------------------------------

MatView mat_alloc(MatShape shape) {
    MatView mat = { NULL, shape };
    if (mat_shape_is_zero(shape)) return mat;
    mat.data = malloc(sizeof(float) * shape.n_rows * shape.n_cols);
    EXPECT_ERRNO(mat.data != NULL);
    return mat;
}

MatView mat_alloc_zero(MatShape shape) {
    MatView mat = mat_alloc(shape);
    if (mat_shape_is_zero(shape)) return mat;
    memset(mat.data, 0, sizeof(float) * shape.n_rows * shape.n_cols);
    return mat;
}

void mat_free(MatView* mat) {
    if (mat == NULL || mat_shape_eq(mat->shape, mat_shape_zero())) return;
    free(mat->data);
    *mat = (MatView) { NULL, mat_shape_zero() };
}

MatView mat_alloc_mul(const MatView* a, const MatView* b) {
    MatShape shape = mat_shape_mul(a->shape, b->shape);
    MatView c = mat_alloc(shape);
    mat_mul(a, b, &c);
    return c;
}

MatView mat_clone(const MatView* a) {
    MatView b = mat_alloc(a->shape);
    memcpy(b.data, a->data, sizeof(float) * a->shape.n_rows * a->shape.n_cols);
    return b;
}
