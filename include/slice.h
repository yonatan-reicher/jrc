// =============================================================================
//          slice.h
// =============================================================================
// A slice is a non-owned pointer to a classic c-style array, and it's length.
// You may access these with `.ptr` and `.len` respectively.
// -----------------------------------------------------------------------------
#pragma once
#include "basic.h"
#include <stddef.h>
#include <string.h>

#define DECLARE_SLICE(T, NAME)                                                 \
    typedef struct NAME {                                                      \
        T* ptr;                                                                \
        size_t len;                                                            \
    } NAME

/// Return a new empty slice.
#define slice_empty() { .len = 0, .ptr = NULL }
/// Return a new slice pointing at some existing data.
#define slice(PTR, LEN) { .len = (LEN), .ptr = (PTR) }
/// Return a new slice pointing at some existing stack array.
#define slice_of_fixed_array(ARRAY) slice((ARRAY), ARRAY_LEN(ARRAY))
/// Return a new slice pointing at some existing dynamic array.
#define slice_of_array(ARRAY) slice((ARRAY)->ptr, (ARRAY)->len)

// ------ Some definitions -----------------------------------------------------

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
