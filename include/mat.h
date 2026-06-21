#pragma once
#include <stddef.h>

// =============================================================================
//                                     mat.h
// =============================================================================
// This module is all about matrices. Matrices are two dimensional, and ordered
// row-wise.
// This module abbreviates "matrix" as "mat".
// -----------------------------------------------------------------------------

typedef struct MatShape {
    size_t n_rows;
    size_t n_cols;
} MatShape;

typedef struct MatView {
    float* data;
    MatShape shape;
} MatView;

/// Creates a new CONSTANT sized matrix with a lifetime of the current scope.
/// The variable arguments should be the data. It is variable arguments because
/// data is supposed to be with curly braces with commas, which gets passed as
/// multiple arguments.
/// The rows and columns should be constant, so the matrix has a constant size.
/// The arguments themselves can be non-constant.
///
/// This cannot initialize an empty matrix. Just set the shape to zero for that.
///
/// ## Example
/// The following defines a 3 x 2 matrix with the first row being [1 2], the
/// second [3 4], the last [5 6].
/// ```c
/// NEW_MAT(my_mat, 3, 2, { 1, 2, 3, 4, 5, 6 });
/// ```
#define NEW_MAT(NAME, N_ROWS, N_COLS, ...) \
    float NAME##_data[N_ROWS * N_COLS] = __VA_ARGS__; \
    MatView NAME = { NAME##_data, { (N_ROWS), (N_COLS) } };

// ------ Matrix Shape ---------------------------------------------------------

MatShape mat_shape_zero(void);

/// Notice that even shapes such as 0 x 5 and 42 x 0 are considered zero shapes.
bool mat_shape_is_zero(MatShape shape);

bool mat_shape_try_mul(MatShape lhs, MatShape rhs, MatShape* out);

MatShape mat_shape_mul(MatShape lhs, MatShape rhs);

bool mat_shape_eq(MatShape a, MatShape b);

MatShape mat_shape_transpose(MatShape shape);

// ------ Matrix Operations ----------------------------------------------------

void mat_mul(const MatView* A, const MatView* B, MatView* C);
/// Multiplies A and B^T, where B^T is the transpose of B.
void mat_mul_transposed(const MatView* a, const MatView* b, MatView* c);

void mat_add(const MatView* a, const MatView* b, MatView* c);
void mat_sub(const MatView* a, const MatView* b, MatView* c);

void mat_scale(const MatView* mat, float scalar, MatView* out);

bool mat_eq(const MatView* A, const MatView* B, float epsilon);

void mat_transpose(const MatView* mat, MatView* out);
void mat_transpose_inplace(MatView* mat);
/// Transpose a matrix that's known to be a vector (either 1 x n or n x 1).
MatView mat_transpose_vector(const MatView* mat);

// ------ Helpers --------------------------------------------------------------

MatView mat_alloc(MatShape shape);

MatView mat_alloc_zero(MatShape shape);

void mat_free(MatView* mat);

MatView mat_alloc_mul(const MatView* A, const MatView* B);

MatView mat_clone(const MatView* mat);
