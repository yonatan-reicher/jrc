// =============================================================================
//          slice.h
// =============================================================================
// A slice is a non-owned pointer to a classic c-style array, and it's length.
// You may access these with `.ptr` and `.len` respectively.
// -----------------------------------------------------------------------------
#pragma once
#include "basic.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define DECLARE_SLICE(T, NAME)                                                 \
    typedef struct NAME {                                                      \
        T* ptr;                                                                \
        size_t len;                                                            \
    } NAME

/// Return a new empty slice.
#define slice_empty() { .len = 0, .ptr = NULL }
/// Return a new slice pointing at some existing data.
#define slice_new(PTR, LEN) { .len = (LEN), .ptr = (PTR) }
/// Return a new slice pointing at some existing stack array.
#define slice_of_fixed_array(ARRAY) slice((ARRAY), ARRAY_LEN(ARRAY))
/// Return a new slice pointing at some existing dynamic array.
#define slice_of_array(ARRAY) slice_new((ARRAY)->ptr, (ARRAY)->len)

/// Expands to the type of the slice's elements.
#define SLICE_ELEMENT_TYPE(NAME)                                               \
    /* We are taking the type of an expression, */                             \
    typeof(  /* And that expression is a dereference of the pointer... */      \
           *(/* to some fake slice of that name. */                            \
             *(NAME*)(void*)0                                                  \
           )                                                                   \
                .ptr                                                           \
    )

/// Returns a new slice literal. The lifetime of the object is the current
/// block's lifetime.
/// Example: slice(VoidPtrSlice, { &a, &b, NULL })
#define slice(NAME, ...)                                                       \
    ((NAME) {                                                                  \
        /* A pointer to some array compound literal. */                        \
        ((SLICE_ELEMENT_TYPE(NAME)[])__VA_ARGS__), /* The literal's length. */ \
        ARRAY_LEN(((SLICE_ELEMENT_TYPE(NAME)[])__VA_ARGS__)),                    \
    })

// ------ Some definitions -----------------------------------------------------

// void
DECLARE_SLICE(void, VoidSlice);
DECLARE_SLICE(const void, ConstVoidSlice);
DECLARE_SLICE(void*, VoidPtrSlice);
DECLARE_SLICE(const void*, ConstVoidPtrSlice);
DECLARE_SLICE(void* const, VoidPtrConstSlice);

// char
DECLARE_SLICE(char, CharSlice);
DECLARE_SLICE(const char, ConstCharSlice);
DECLARE_SLICE(char*, CharPtrSlice);
DECLARE_SLICE(const char*, ConstCharPtrSlice);

// ------ Some string functions! -----------------------------------------------

CharSlice slice_of_str(char* str);
ConstCharSlice slice_of_const_str(const char* str);
// Integers
#define DECLARE_INT_SLICES(BITS)                                               \
    DECLARE_SLICE(uint##BITS##_t, UInt##BITS##Slice);                          \
    DECLARE_SLICE(const uint##BITS##_t, ConstUInt##BITS##Slice);               \
    DECLARE_SLICE(int##BITS##_t, Int##BITS##Slice);                            \
    DECLARE_SLICE(const int##BITS##_t, ConstInt##BITS##Slice)
DECLARE_INT_SLICES(8);
DECLARE_INT_SLICES(16);
DECLARE_INT_SLICES(32);
DECLARE_INT_SLICES(64);
