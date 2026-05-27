#pragma once
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
    // VALUE_FUNC,
} ValueKind;

typedef struct Value {
    ValueKind kind;
    union {
        int64_t i;
    } data;
} Value;

Value value_int(int64_t);

Value value_null(void);

char* value_to_str(Value);
