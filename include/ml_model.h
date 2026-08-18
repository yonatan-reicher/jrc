#pragma once
#include "mat.h"

/// An interface for machine learning models. Instances support mapping inputs
/// to outputs, deriving input errors from output errors, and training on output
/// errors.
typedef struct MLModel {
    void (*forward)(const void* self, const MatView* inp, MatView* out);
    void (*backward)(
        const void* self,
        const MatView* inp,
        const MatView* out_err,
        MatView* inp_err
    );
    void (*train)(
        void* self, const MatView* inp, const MatView* out_err, float lr
    );
    bool (*supports_inp_shape)(const void* self, MatShape inp_shape);
    MatShape (*out_shape)(const void* self, MatShape inp_shape);
} MLModel;
