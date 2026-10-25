#include "stream.h"

// ------ Stream implementation ------------------------------------------------

// Abstract Functions

void stream_free(Stream* s) {
    switch (s->kind) {
#define X(SNAKE, PASCAL, UPPER)                                                \
    case STREAM_KIND_##UPPER: SNAKE##_stream_free(&s->data.SNAKE); break;
        X_ON_STREAM_KINDS
#undef X
    }
}

Stream stream_clone(const Stream* s) {
    switch (s->kind) {
#define X(SNAKE, PASCAL, UPPER)                                                \
    case STREAM_KIND_##UPPER: return SNAKE##_stream_clone(&s->data.SNAKE);
        X_ON_STREAM_KINDS
#undef X
    }
}

size_t stream_read(Stream* st, UInt8Slice sl) {
    switch (st->kind) {
#define X(SNAKE, PASCAL, UPPER)                                                \
    case STREAM_KIND_##UPPER: return SNAKE##_stream_read(&st->data.SNAKE, sl);
        X_ON_STREAM_KINDS
#undef X
    }
}

// Constructors

Stream stream_empty() {
    return (Stream) { STREAM_KIND_EMPTY, { .empty = empty_stream() } };
}

Stream stream_of_slice(ConstUInt8Slice s) {
    if (s.len == 0) return stream_empty();
    return (Stream) { STREAM_KIND_SLICE, { .slice = slice_stream(s) } };
}

// ------ Empty Stream ---------------------------------------------------------

EmptyStream empty_stream() {
    return 0;
}

void empty_stream_free(EmptyStream*) {
    // nothing
}

Stream empty_stream_clone(const EmptyStream*) {
    return stream_empty();
}

size_t empty_stream_read(EmptyStream*, UInt8Slice) {
    return 0;
}

// ------ Slice Stream ---------------------------------------------------------

SliceStream slice_stream(ConstUInt8Slice s) {
    return (SliceStream) { s, 0 };
}

void slice_stream_free(SliceStream*) {
    // nothing
}

Stream slice_stream_clone(const SliceStream* s) {
    return (Stream) {
        STREAM_KIND_SLICE,
        { .slice = *s },
    };
}

size_t slice_stream_read(SliceStream* st, UInt8Slice sl) {
    size_t n_left = st->slice.len - st->index;
    size_t n_read = MIN(sl.len, n_left);
    memcpy(sl.ptr, st->slice.ptr + st->index, n_left);
    st->index += n_read;
    return n_read;
}
