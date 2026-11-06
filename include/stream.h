#pragma once

// =============================================================================
//                                    Stream
// =============================================================================
// A Stream is a series of bytes. Streams can be lazy, can be infinite, and can
// contain zero-bytes (unlike null-terminated strings).
// -----------------------------------------------------------------------------

#include "array.h"
#include "slice.h"
#include <stdint.h>
#include <stdlib.h>

// ------ Helpers --------------------------------------------------------------

// slices
DECLARE_SLICE(struct Stream, StreamSlice);
DECLARE_SLICE(const struct Stream, ConstStreamSlice);
DECLARE_SLICE(struct Stream*, StreamPtrSlice);
DECLARE_SLICE(const struct Stream*, ConstStreamPtrSlice);
DECLARE_SLICE(struct Stream* const, StreamPtrConstSlice);
DECLARE_SLICE(const struct Stream* const, ConstStreamPtrConstSlice);
// array
DECLARE_ARRAY(struct Stream, StreamArray);
DECLARE_ARRAY(struct Stream*, StreamPtrArray);
DECLARE_ARRAY(const struct Stream*, ConstStreamPtrArray);

// ------ API ------------------------------------------------------------------

typedef struct Stream Stream;

// Abstract functions (functions dispatched based on stream kind)

/// Free the given streams.
void stream_free(Stream*);
/// Clone the stream.
Stream stream_clone(Stream*);
/// Read up-to the specified amount of bytes from a stream. Returns the actual
/// amount read.
size_t stream_read(Stream*, UInt8Slice);

// Constructors

/// Create a new empty stream.
Stream stream_empty();
/// Create a stream that views into a slice.
Stream stream_of_slice(ConstUInt8Slice);
/// Create a stream that views into a string.
Stream stream_of_str(const char*);
/// Create a stream that owns an array.
Stream stream_from_array(UInt8Array);
/// Create a stream that owns a string.
Stream stream_from_str(char*);
/// Create a stream that copies a string.
Stream stream_of_str_copy(const char*);
Stream stream_from_file(FILE*);
/// Create a streams from the standard output and standard error of a command,
/// given some input stream. The command can be an absolute path, a relative
/// path, or a name from the search path.
///
/// In the case where the operation is successful, `true` is
/// returned. Otherwise, `false` is returned and the `errno` is set accordingly.
///
/// Both the output and the error streams can be `NULL`. In that case, they are
/// redirected to nothing. If both are `NULL`, this function call blocks until
/// the command completes execution.
bool stream_cmd_inp_out_err(
    const char* name,
    ConstCharPtrSlice args,
    Stream inp,
    Stream* out,
    Stream* err
);
/// Create a stream from the output of a command.
/// Panics if something went wrong.
Stream stream_cmd_out(const char* name, ConstCharPtrSlice args);
/// Create a stream from the output of a command, with the given stream as the
/// input.
/// Panics if something went wrong.
Stream stream_cmd_inp_out(const char* name, ConstCharPtrSlice args, Stream inp);
Stream stream_concat_2(Stream, Stream);
Stream stream_concat_3(Stream, Stream, Stream);
Stream stream_concat_4(Stream, Stream, Stream, Stream);
Stream stream_concat(StreamArray);

// Reading

UInt8Array stream_read_to_array(Stream*);
UInt8Array stream_to_array(Stream);

char* stream_read_to_str(Stream*);
char* stream_to_str(Stream);

bool stream_read_to_file(Stream*, FILE*);

#define STREAM_READ_BY_CHUNKS(S, BUF_SIZE, VAR)                                \
    uint8_t VAR##_buf[BUF_SIZE];                                               \
    size_t VAR##_n_read = 0;                                                   \
    UInt8Slice VAR##_full = slice_of_fixed_array(VAR##_buf);                   \
    UInt8Slice VAR = slice_empty();                                            \
    while (                                                                    \
        (VAR##_n_read = stream_read((S), VAR##_full),                          \
         (VAR) = slice_take(VAR##_full, VAR##_n_read),                         \
         VAR##_n_read != 0)                                                    \
    )

// TODO: add
// - stream concat
// - stream concat all
// - stream char
// - stream trim
// - stream take
// - stream drop

// ------ Stream Name Table ----------------------------------------------------
// We have a macro holding the name of all kinds of streams in a sort of table
// where rows are applied onto a macro called `X`. To use it, define `X` and
// insert the table.

#define X_ON_STREAM_KINDS                                                      \
    /* X(snake case name, pascal case, upper case, clone kind) */              \
    /* The clone type is either `NORMAL` for a simple clone signature and */   \
    /* `ADVANCED` for a stranger one. */                                       \
    X(empty, Empty, EMPTY, NORMAL)                                             \
    X(slice, Slice, SLICE, NORMAL)                                             \
    X(array, Array, ARRAY, NORMAL)                                             \
    X(str, Str, STR, NORMAL)                                                   \
    X(from_str, FromStr, FROM_STR, NORMAL)                                     \
    X(file, File, FILE, ADVANCED)                                              \
    X(shared_file, SharedFile, SHARED_FILE, NORMAL)                            \
    X(buffered, Buffered, BUFFERED, NORMAL)                                    \
    X(cmd, Cmd, CMD, ADVANCED)                                                 \
    X(concat, Concat, CONCAT, NORMAL)

// ------ Boolean Operators ----------------------------------------------------

// TODO: Should these be taken by reference and cloned?
bool stream_starts_with(Stream, Stream);

// ------ Boolean Operators ----------------------------------------------------

// TODO: Should these be taken by reference and cloned?
bool stream_starts_with(Stream, Stream);

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

// Array

typedef struct ArrayStream {
    uint8_t* ptr;
    size_t len;
    size_t index;
} ArrayStream;

ArrayStream array_stream(UInt8Array);

// String

typedef struct StrStream {
    const char* str;
    size_t index;
} StrStream;

StrStream str_stream(const char*);

// From String

typedef struct FromStrStream {
    char* str;
    size_t index;
} FromStrStream;

FromStrStream from_str_stream(char*);

// File

typedef struct FileStream {
    FILE* file;
} FileStream;

FileStream file_stream(FILE*);

// Shared File

typedef struct SharedFileStream {
    // This field could be stored in the box, because it's shared. But we don't
    // need to!
    FILE* file;
    struct SharedFileStreamCursorInfo* info;
    size_t* rc;
} SharedFileStream;

typedef struct SharedFileStreamCursorInfo {
    fpos_t pos;
    bool eof_reached;
    bool had_err;
} SharedFileStreamCursorInfo;

SharedFileStream shared_file_stream_from_file_stream(FileStream);

// Buffered

/// This is a wrapper around a stream, making it pretty-cheaply clone-able.
/// This is done by reference-counting the original stream, and storing output
/// in a buffer for copies to read from.
typedef struct BufferedStream {
    size_t index;
    struct BufferedStreamBox* box;
} BufferedStream;

BufferedStream buffered_stream_from_stream(Stream);

// Cmd

typedef enum CmdStreamKind {
    CMD_STREAM_OUT,
    CMD_STREAM_ERR,
} CmdStreamKind;

typedef struct CmdStream {
    struct CmdStreamData* data;
    CmdStreamKind kind;
} CmdStream;

bool cmd_stream(
    const char* name,
    ConstCharPtrSlice args,
    Stream inp,
    CmdStream* out,
    CmdStream* err
);

// Concat

typedef struct ConcatStream {
    size_t n;
    Stream* sub_streams;
    size_t index;
} ConcatStream;

ConcatStream concat_stream(StreamArray);

// ------ Stream Struct --------------------------------------------------------

typedef enum StreamKind {
#define X(SNAKE, PASCAL, UPPER, CLONE_KIND) STREAM_KIND_##UPPER,
    X_ON_STREAM_KINDS
#undef X
} StreamKind;

typedef struct Stream {
    StreamKind kind;
    union {
#define X(SNAKE, PASCAL, UPPER, CLONE_KIND) PASCAL##Stream SNAKE;
        X_ON_STREAM_KINDS
#undef X
    } data;
} Stream;

// ------ Define Stream Kind Abstract Functions --------------------------------

#define STREAM_CLONE_SIGNATURE_NORMAL(SNAKE, PASCAL, UPPER)                    \
    PASCAL##Stream SNAKE##_stream_clone(PASCAL##Stream*)

#define STREAM_CLONE_SIGNATURE_ADVANCED(SNAKE, PASCAL, UPPER)                  \
    Stream SNAKE##_stream_clone(enum StreamKind*, PASCAL##Stream*)

#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    void SNAKE##_stream_free(PASCAL##Stream*);                                 \
    STREAM_CLONE_SIGNATURE_##CLONE_KIND(SNAKE, PASCAL, UPPER);                 \
    size_t SNAKE##_stream_read(PASCAL##Stream*, UInt8Slice);
X_ON_STREAM_KINDS
#undef X

// ------ Conversions ----------------------------------------------------------

#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    Stream stream_from_##SNAKE##_stream(PASCAL##Stream);
X_ON_STREAM_KINDS
#undef X
