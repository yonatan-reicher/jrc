#include "type.h"
#include "str.h"
#include <stdlib.h>

Type type_null(void) { return (Type) { TYPE_NULL, {} }; }
Type type_error() { return (Type) { TYPE_ERROR, {} }; }
Type type_int(void) { return (Type) { TYPE_INT, {} }; }

void type_free(Type* self) {
    switch (self->kind) {
        case TYPE_NULL: break;
        case TYPE_INT: break;
        case TYPE_ERROR: break;
    }
    *self = (Type) { 0 };
}

char* type_to_str(const Type*) {
    return str_clone("int");
}

bool type_eq(const Type* a, const Type* b) {
    if (a->kind != b->kind) return false;
    switch (a->kind) {
        case TYPE_NULL: return true;
        case TYPE_ERROR: return true;
        case TYPE_INT: return true;
    }
}
