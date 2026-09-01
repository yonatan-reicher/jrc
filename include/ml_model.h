#pragma once
#include "mat.h"

typedef void MLModelForward(const void* self, const MatView* inp, MatView* out);
typedef void MLModelBackward(
    const void* self,
    const MatView* inp,
    const MatView* out_err,
    MatView* inp_err
);
typedef void MLModelTrain(
    void* self, const MatView* inp, const MatView* out_err, float lr
);
typedef bool MLModelSupportsInpShape(const void* self, MatShape inp_shape);
typedef MatShape MLModelOutShape(const void* self, MatShape inp_shape);

/// An interface for machine learning models. Instances support mapping inputs
/// to outputs, deriving input errors from output errors, and training on output
/// errors.
typedef struct MLModel {
    MLModelForward* forward;
    MLModelBackward* backward;
    MLModelTrain* train;
    MLModelSupportsInpShape* supports_inp_shape;
    MLModelOutShape* out_shape;
} MLModel;
