#pragma once
#include "ast.h"

typedef enum ValueKind {
    VALUE_INT,
} ValueKind;

// TODO: Move to value.h
typedef struct Value {
    ValueKind kind;
    union {
        int64_t i;
    } data;
} Value;

Value value_int(int64_t);

Value eval(const Ast* ast);
