#pragma once
// TODO: #include "ast.h"
#include "parser.h"

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

Value eval(const Ast* ast);
