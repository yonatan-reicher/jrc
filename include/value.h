#pragma once
#include <stdint.h>

// =============================================================================
//                                    Value.h
// =============================================================================
// This module introduces `Value`, which is a generic datatype that can hold all
// kinds of things, fit for dynamic operations. Made for the interpreter module.
// Currently only supports integers, but this will change.
// -----------------------------------------------------------------------------

typedef enum ValueKind {
    VALUE_INT,
} ValueKind;

typedef struct Value {
    ValueKind kind;
    union {
        int64_t i;
    } data;
} Value;

Value value_int(int64_t);

char* value_to_str(Value);
