// =============================================================================
//      Array.h
// =============================================================================
// This header files defines macros and functions for working with arrays in a
// more modern way. Start with defining an array with `DEFINE_ARRAY(T)`. This
// defines the `Array(T)` type for you.
//
// ---- In this module ---------------------------------------------------------
// 
// `DEFINE_ARRAY(T)` - Define the array of type T. This will make a compilation
// error if it had already been defined for the type, so use an define guard or
// place all your invocations next to one another.
//
// `Array(T)` - The type name of the array of the given type.
//
// `array.len` - Every array has this field.
// -----------------------------------------------------------------------------

#pragma once
#include <stddef.h>

/// `Array(T)` is the type of arrays of `T`.
#define Array(T) _Array_##T

/// Define the array of `T`.
#define DEFINE_ARRAY(T) \
    typedef struct Array(T) { size_t len; T *ptr; } Array(T)

/// Return a new empty array of the given type.
#define array_empty(T) ((Array(T)) { .len = 0, .ptr = NULL })

// #define array_add(A, X)
//     do {
//         
