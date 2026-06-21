#include "mat.h"
#include "basic.h"
#include <stdlib.h>

void test_empty_mul() {
    MatView a = { NULL, { 0, 0 } };
    MatView b = { NULL, { 0, 0 } };
    MatShape out_shape = mat_shape_mul(a.shape, b.shape);
    EXPECT(
        mat_shape_eq(out_shape, (MatShape) { 0, 0 }),
        "expected empty output matrix"
    );
}

void test_a_generic_multiplication() {
    NEW_MAT(a, 3, 2, { 1, 2, 3, 4, 5, 6 });
    NEW_MAT(b, 2, 4, { 7, 8, 9, 0, 1, 2, 3, 4 });
    //
    //   [ 1 2 ]     [ 7 8 9 0 ]        [  9 12 15  8 ]
    //   [ 3 4 ]  ·  [ 1 2 3 4 ]    =   [ 25 32 39 16 ]
    //   [ 5 6 ]                        [ 41 52 63 24 ]
    NEW_MAT(expected, 3, 4, { 9, 12, 15, 8, 25, 32, 39, 16, 41, 52, 63, 24 });
    MatView output = mat_alloc_mul(&a, &b);
    EXPECT(
        mat_eq(&output, &expected, 0.001f), "matrix multiplication wrong output"
    );
}

int main(void) {
    test_empty_mul();
    return 0;
}
