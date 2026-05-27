#pragma once

#include <stdbool.h>

typedef enum TypeKind {
    TYPE_NULL,
    TYPE_ERROR,
    TYPE_INT,
} TypeKind;

typedef struct Type {
    TypeKind kind;
    union {
        int placeholder;
    } data;
} Type;

Type type_null(void);
Type type_error();
Type type_int(void);

void type_free(Type*);

char* type_to_str(const Type*);

bool type_eq(const Type*, const Type*);
