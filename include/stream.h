#pragma once

// =============================================================================
//                                    Stream
// =============================================================================
// A Stream is a series of bytes. Streams can be lazy, can be infinite, and can
// contain zero-bytes (unlike null-terminated strings).
// -----------------------------------------------------------------------------

#include "slice.h"
#include <stdint.h>
#include <stdlib.h>

// ------ API ------------------------------------------------------------------

typedef struct Stream Stream;

// Abstract functions (functions dispatched based on stream kind)

/// Free the given streams.
void stream_free(Stream*);
/// Clone the stream.
Stream stream_clone(const Stream*);
/// Read up-to the specified amount of bytes from a stream. Returns the actual
/// amount read.
size_t stream_read(Stream*, UInt8Slice);

// Constructors

Stream stream_empty();
Stream stream_of_slice(ConstUInt8Slice);

// ------ Stream Name Table ----------------------------------------------------
// We have a macro holding the name of all kinds of streams in a sort of table
// where rows are applied onto a macro called `X`. To use it, define `X` and
// insert the table.

#define X_ON_STREAM_KINDS                                                      \
    X(empty, Empty, EMPTY)                                                     \
    X(slice, Slice, SLICE)

// ------ Stream Kind Definitions ----------------------------------------------

// Empty

typedef int EmptyStream;

EmptyStream empty_stream();

// Slice

typedef struct SliceStream {
    ConstUInt8Slice slice;
    size_t index;
} SliceStream;

SliceStream slice_stream(ConstUInt8Slice);

// Definitions for all

#define X(SNAKE, PASCAL, UPPER)                                                \
    void SNAKE##_stream_free(PASCAL##Stream*);                                 \
    Stream SNAKE##_stream_clone(const PASCAL##Stream*);                        \
    size_t SNAKE##_stream_read(PASCAL##Stream*, UInt8Slice);
X_ON_STREAM_KINDS
#undef X

// ------ Stream Struct --------------------------------------------------------

typedef enum StreamKind {
#define X(SNAKE, PASCAL, UPPER) STREAM_KIND_##UPPER,
    X_ON_STREAM_KINDS
#undef X
} StreamKind;

typedef struct Stream {
    StreamKind kind;
    union {
#define X(SNAKE, PASCAL, UPPER) PASCAL##Stream SNAKE;
        X_ON_STREAM_KINDS
#undef X
    } data;
} Stream;
