// =============================================================================
//                                    Array.h
// =============================================================================
// This header file defines macros and functions for working with arrays in a
// more modern way. Array types can be defined with `DECLARE_ARRAY(T, Name)`
// (e.g. `DECLARE_ARRAY(char, CharArray)`). `T` must be a mutable type. You can
// then use the `Name` type. It has fields `.ptr`, `.len` and `.cap`.
//
// Use `array_empty()` to get a new empty array. You may need to cast it first
// to the correct type (e.g. `(CharArray) array_empty()`). Use `array_push` to
// add elements to the array. Finally, call `array_free` when you are done.
// -----------------------------------------------------------------------------

#pragma once
#include "basic.h"
#include <stddef.h>
#include <stdint.h>

#define DECLARE_ARRAY(T, Name)                                                 \
    typedef struct Name {                                                      \
        T* ptr;                                                                \
        size_t len;                                                            \
        size_t cap;                                                            \
    } Name

/// Return a new empty array of the given type.
#define array_empty() { .len = 0, .cap = 0, .ptr = NULL }

#define array_push(ARR, ...)                                                   \
    do {                                                                       \
        /* Avoid calling `realloc` when capacity is 0. */                      \
        if ((ARR)->cap == 0) {                                                 \
            (ARR)->cap = MAX(4, (ARR)->len + 1);                               \
            (ARR)->ptr = malloc((ARR)->cap * sizeof((ARR)->ptr[0]));           \
        } else if ((ARR)->len == (ARR)->cap) {                                 \
            (ARR)->cap *= 2;                                                   \
            (ARR)->ptr =                                                       \
                realloc((ARR)->ptr, (ARR)->cap * sizeof((ARR)->ptr[0]));       \
        }                                                                      \
        (ARR)->ptr[(ARR)->len++] = (__VA_ARGS__);                              \
    } while (0)

#define array_free(ARR)                                                        \
    do {                                                                       \
        free((ARR)->ptr);                                                      \
        (ARR)->ptr = NULL;                                                     \
        (ARR)->len = 0;                                                        \
    } while (0)

/// Empty the array without freeing the memory.
#define array_clear(ARR)                                                       \
    do {                                                                       \
        (ARR)->len = 0;                                                        \
    } while (0)

#define array_get(ARR, IDX)                                                    \
    ((IDX) >= (ARR)->len                                                       \
         ? (PANIC("index %zu/%zu out of bounds", (IDX), (ARR)->len),           \
            (ARR)->ptr)                                                        \
         : &(ARR)->ptr[IDX])

#define array_pop(ARR)                                                         \
    do {                                                                       \
        if ((ARR)->len == 0) PANIC("cannot pop from empty array");             \
        (ARR)->len--;                                                          \
    } while (0)

#define array_first(ARR)                                                       \
    ((ARR)->len == 0                                                           \
         ? (PANIC("cannot get first element of empty array"), (ARR)->ptr[0])   \
         : (ARR)->ptr[0])
#define array_last(ARR) ((ARR)->ptr[(ARR)->len - 1])

#define array_extend(ARR, PTR, LEN)                                            \
    do {                                                                       \
        /* TODO: reserve capacity first. */                                    \
        for (size_t i = 0; i < (LEN); i++) array_push((ARR), (PTR)[i]);        \
    } while (0)

#define ARRAY_FOREACH(ARR, VAR)                                                \
    for (size_t i_##VAR = 0; i_##VAR < (ARR)->len; i_##VAR++)                  \
        for (typeof_unqual(&(ARR)->ptr[0]) VAR = &(ARR)->ptr[i_##VAR];         \
             VAR != NULL;                                                      \
             VAR = NULL)

/// A reversed version of `ARRAY_FOREACH`. The index variable still counts up.
#define ARRAY_FOREACH_REV(ARR, VAR)                                            \
    for (size_t i_##VAR = 0; i_##VAR < (ARR)->len; i_##VAR++)                  \
        for (typeof_unqual(&(ARR)->ptr[0])                                     \
                 VAR = &(ARR)->ptr[(ARR)->len - 1 - i_##VAR];                  \
             VAR != NULL;                                                      \
             VAR = NULL)

// =============================================================================
//                            Some Array Declarations
// =============================================================================

DECLARE_ARRAY(void*, VoidPtrArray);
DECLARE_ARRAY(char, CharArray);
DECLARE_ARRAY(char*, CharPtrArray);
DECLARE_ARRAY(const char*, ConstCharPtrArray);
// Integers
#define DECLARE_INT_ARRAYS(BITS)                                               \
    DECLARE_ARRAY(uint##BITS##_t, UInt##BITS##Array);                          \
    DECLARE_ARRAY(int##BITS##_t, Int##BITS##Array);
DECLARE_INT_ARRAYS(8)
DECLARE_INT_ARRAYS(16)
DECLARE_INT_ARRAYS(32)
DECLARE_INT_ARRAYS(64)
