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

/// The name of the type of slices of the given type.
#define Slice(T) _Slice_##T
/// The name of the type of slices of pointers to the given type.
#define PtrSlice(T) _SliceOfPtr_##T

/// Define the slice of the given type.
#define DEFINE_SLICE(T) \
    typedef struct Slice(T) { size_t len; T *ptr; } Slice(T)

#define DEFINE_PTR_SLICE(T) \
    typedef struct PtrSlice(T) { size_t len; T **ptr; } PtrSlice(T)

/// Return a new empty slice.
#define slice_empty(T) ((Slice(T)) { .len = 0, .ptr = NULL })
#define ptr_slice_empty(T) ((PtrSlice(T)) { .len = 0, .ptr = NULL })

#define slice(T, LEN, PTR) ((Slice(T)) { .len = (LEN), .ptr = (PTR) })
#define ptr_slice(T, LEN, PTR) ((PtrSlice(T)) { .len = (LEN), .ptr = (PTR) })

// ------ Some string functions! -----------------------------------------------

DEFINE_SLICE(char);

inline Slice(char) slice_of_str(char *str) {
    size_t len = strlen(str);
    return slice(char, len, str);
}
