#pragma once

#include <stdbool.h>

typedef enum TypeKind {
    /// This is not the type of a null value, but the lack of a type altogether.
    TYPE_NULL,
    /// Not an error type, but an error where a type is expected.
    TYPE_ERROR,
    /// The type of integers.
    TYPE_INT,
} TypeKind;

typedef struct Type {
    TypeKind kind;
    union {
        int placeholder;
    } data;
} Type;

Type type_null(void);
Type type_error(void);
Type type_int(void);

void type_free(Type*);

char* type_to_str(const Type*);

bool type_eq(const Type*, const Type*);
