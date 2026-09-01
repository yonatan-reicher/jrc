#pragma once

#include "slice.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum Endianness : bool {
    ENDIANNESS_BIG,
    ENDIANNESS_LIL,
} Endianness;

/// Get the endianness of the machine.
Endianness endianness_get();
/// Are we big-endian-ing?
bool endianness_is_big();
/// Are we little-endian-ing?
bool endianness_is_lil();

/// "Toggle" the endianness of a byte sequence. That is, reverse the bytes in
/// the slice.
void endianness_reverse(UInt8Slice);

/// Convert bytes to the given endianness.
void endianness_to(UInt8Slice, Endianness);
/// Convert bytes from the given endianness to the machine endianness.
void endianness_of(UInt8Slice, Endianness);

/// Convert the given slice to big endian.
void endianness_to_big(UInt8Slice);
/// Convert the given slice to little endian.
void endianness_to_lil(UInt8Slice);

/// Convert the given slice to machine endianness from big endian.
void endianness_of_big(UInt8Slice);
/// Convert the given slice to machine endianness from little endian.
void endianness_of_lil(UInt8Slice);
