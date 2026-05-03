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

// ------ Some definitions -----------------------------------------------------

DECLARE_SLICE(char, CharSlice);
DECLARE_SLICE(const char, ConstCharSlice);
DECLARE_SLICE(char*, CharPtrSlice);
DECLARE_SLICE(const char*, ConstCharPtrSlice);

// ------ Some string functions! -----------------------------------------------

CharSlice slice_of_str(char* str);
ConstCharSlice slice_of_const_str(const char* str);
