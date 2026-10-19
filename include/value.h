#pragma once
#include "ast.h"
#include "slice.h"
#include <stdint.h>

// =============================================================================
//                                    Value.h
// =============================================================================
// This module introduces `Value`, which is a generic datatype that can hold all
// kinds of things, fit for dynamic operations. Made for the interpreter module.
// Supported kinds of values:
// - Integers
// - Functions without arguments and without a closure
// -----------------------------------------------------------------------------

typedef enum ValueKind {
    /// This is not for a value representing null, but for when no value is
    /// available.
    VALUE_NULL, // TODO: rename to VALUE_KIND_NULL or something
    VALUE_INT,
    VALUE_FUNC,
} ValueKind;

typedef struct Value {
    ValueKind kind;
    union {
        int64_t i;
        struct ValueFunc* f;
    } data;
} Value;

typedef struct ValueFuncCapture {
    char* var_name;
    Value value;
} ValueFuncCapture;

DECLARE_SLICE(ValueFuncCapture, ValueFuncCaptureSlice);
DECLARE_SLICE(const ValueFuncCapture, ConstValueFuncCaptureSlice);

typedef struct ValueFunc {
    uint64_t rc;
    char* param_name;
    const Ast* body;
    size_t n_captured;
    ValueFuncCapture captured[];
} ValueFunc;

void value_inc_rc(Value);
void value_dec_rc(Value);

Value value_int(int64_t);

Value value_null(void);

/// The parameter name and the captured names are copied. The AST is referenced.
/// The values are moved.
Value value_func_new(
    const char* param_name,
    Ast* body,
    size_t n_captured,
    const char* const captured_variable_names[],
    Value captured_values[]
);
ValueFuncCaptureSlice value_func_captured(Value);

char* value_to_str(Value);
