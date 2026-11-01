#include "stream.h"
#include "slice.h"
#include "str.h"

// ------ Stream implementation ------------------------------------------------

// Abstract Functions

void stream_free(Stream* s) {
    switch (s->kind) {
#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    case STREAM_KIND_##UPPER: SNAKE##_stream_free(&s->data.SNAKE); break;
        X_ON_STREAM_KINDS
#undef X
    }
}

Stream stream_clone(Stream* s) {
    switch (s->kind) {
#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    CLONE_##CLONE_KIND(SNAKE, PASCAL, UPPER)
#define CLONE_NORMAL(SNAKE, PASCAL, UPPER)                                     \
    case STREAM_KIND_##UPPER:                                                  \
        return stream_from_##SNAKE##_stream(                                   \
            SNAKE##_stream_clone(&s->data.SNAKE)                               \
        );
#define CLONE_ADVANCED(SNAKE, PASCAL, UPPER)                                   \
    case STREAM_KIND_##UPPER:                                                  \
        return SNAKE##_stream_clone(&s->kind, &s->data.SNAKE);
        /* Call X */
        X_ON_STREAM_KINDS
#undef X
#undef CLONE_NORMAL
#undef CLONE_ADVANCED
    }
}

size_t stream_read(Stream* st, UInt8Slice sl) {
    ASSERT(sl.len > 0, "when reading from stream, must give a non-empty slice");
    switch (st->kind) {
#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    case STREAM_KIND_##UPPER: return SNAKE##_stream_read(&st->data.SNAKE, sl);
        X_ON_STREAM_KINDS
#undef X
    }
}

// Constructors

Stream stream_empty() {
    return stream_from_empty_stream(empty_stream());
}

Stream stream_of_slice(ConstUInt8Slice s) {
    if (s.len == 0) return stream_empty();
    return stream_from_slice_stream(slice_stream(s));
}

Stream stream_of_str(const char* s) {
    if (s == NULL || s[0] == 0) return stream_empty();
    return stream_from_str_stream(str_stream(s));
}

Stream stream_from_array(UInt8Array a) {
    if (a.len == 0) {
        array_free(&a);
        return stream_empty();
    }
    return stream_from_array_stream(array_stream(a));
}

Stream stream_from_str(char* s) {
    if (s == NULL || s[0] == 0) {
        free(s);
        return stream_empty();
    }
    // This is an unfortunate name, isn't it?
    return stream_from_from_str_stream(from_str_stream(s));
}

Stream stream_of_str_copy(const char* s) {
    UInt8Array a = array_empty();
    array_extend_raw(&a, (const uint8_t*)s, strlen(s));
    return stream_from_array(a);
}

Stream stream_from_file(FILE* f) {
    return stream_from_file_stream(file_stream(f));
}

// Reading

UInt8Array stream_read_to_array(Stream* s) {
    UInt8Array ret = array_empty();
    STREAM_READ_BY_CHUNKS(s, 1024, slice) {
        // This actually extends the array's memory exactly by the needed
        // amount. If we want to be more performant, we could double it, or
        // something else that's smart.
        array_extend(&ret, slice);
    }
    return ret;
}

UInt8Array stream_to_array(Stream s) {
    UInt8Array a = stream_read_to_array(&s);
    stream_free(&s);
    return a;
}

char* stream_read_to_str(Stream* s) {
    // Who has time to actually implement stuff?
    UInt8Array a = stream_read_to_array(s);
    array_push(&a, '\0');
    // Assumes `array_free` just calls `free`...
    return (char*)a.ptr;
}

char* stream_to_str(Stream s) {
    char* str = stream_read_to_str(&s);
    stream_free(&s);
    return str;
}

bool stream_read_to_file(Stream* s, FILE* f) {
    STREAM_READ_BY_CHUNKS(s, 1024, slice) {
        size_t n_wrote = fwrite(slice.ptr, 1, slice.len, f);
        if (n_wrote < slice.len) return false;
    }
    return true;
}

// ------ Empty Stream ---------------------------------------------------------

EmptyStream empty_stream() {
    return 0;
}

void empty_stream_free(EmptyStream*) {
    // nothing
}

EmptyStream empty_stream_clone(EmptyStream*) {
    return empty_stream();
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

SliceStream slice_stream_clone(SliceStream* s) {
    return *s;
}

size_t slice_stream_read(SliceStream* st, UInt8Slice sl) {
    size_t n_left = st->slice.len - st->index;
    size_t n_read = MIN(sl.len, n_left);
    memcpy(sl.ptr, st->slice.ptr + st->index, n_left);
    st->index += n_read;
    return n_read;
}

// ------ Array Stream ---------------------------------------------------------

ArrayStream array_stream(UInt8Array s) {
    return (ArrayStream) { s.ptr, s.len, 0 };
}

void array_stream_free(ArrayStream* s) {
    // This assumes that `array_free` just calls `free` on the pointer. Which,
    // as of writing this, it does.
    free(s->ptr);
}

ArrayStream array_stream_clone(ArrayStream* s) {
    // TODO: This is a bug.
    return *s;
}

size_t array_stream_read(ArrayStream* st, UInt8Slice sl) {
    size_t n_left = st->len - st->index;
    size_t n_read = MIN(sl.len, n_left);
    memcpy(sl.ptr, st->ptr + st->index, n_left);
    st->index += n_read;
    return n_read;
}

// ------ String Stream --------------------------------------------------------

StrStream str_stream(const char* s) {
    return (StrStream) { s, 0 };
}

void str_stream_free(StrStream*) {
    // nothing
}

StrStream str_stream_clone(StrStream* s) {
    return *s;
}

size_t str_stream_read(StrStream* st, UInt8Slice sl) {
    size_t i = 0;
    while (true) {
        if (!(i < sl.len)) break; // can we write?
        char c = st->str[st->index + i];
        if (c == 0) break; // can we read?
        sl.ptr[i] = (uint8_t)st->str[st->index + i];
        i++;
    }
    st->index += i;
    return i;
}

// ------ From-String Stream ---------------------------------------------------

FromStrStream from_str_stream(char* s) {
    return (FromStrStream) { s, 0 };
}

void from_str_stream_free(FromStrStream* s) {
    free(s->str);
    s->str = NULL;
}

FromStrStream from_str_stream_clone(FromStrStream* s) {
    return from_str_stream(str_clone(s->str));
}

size_t from_str_stream_read(FromStrStream* st, UInt8Slice sl) {
    StrStream my_str_stream = str_stream(st->str);
    size_t n_read = str_stream_read(&my_str_stream, sl);
    st->index += n_read;
    return n_read;
}

// ------ File -----------------------------------------------------------------

FileStream file_stream(FILE* f) {
    return (FileStream) { f };
}

void file_stream_free(FileStream* f) {
    fclose(f->file);
    return;
}

Stream file_stream_clone(StreamKind* kind, FileStream* s) {
    SharedFileStream* ss = (SharedFileStream*)s;
    *kind = STREAM_KIND_SHARED_FILE;
    *ss = shared_file_stream_from_file_stream(*s);
    return stream_from_shared_file_stream(shared_file_stream_clone(ss));
}

size_t file_stream_read(FileStream* s, UInt8Slice buf) {
    if (buf.len == 0) return 0;
start:
    FILE* const f = s->file;
    if (feof(f)) return 0;
    if (ferror(f)) {
        PANIC(
            "something went wrong - file stream with ptr %p had it's error "
            "flag set",
            (void*)f
        );
    }
    size_t n_read = fread(buf.ptr, 1, buf.len, f);
    if (n_read == 0) goto start;
    return n_read;
}

// ------ Shared File Stream ---------------------------------------------------

SharedFileStream shared_file_stream_from_file_stream(FileStream s) {
    FILE* f = s.file;
    fpos_t pos;
    if (fgetpos(f, &pos)) { // error
        // TODO: handle the error by creating a shared buffer instead of a
        // seeking.
        PANIC("not supported yet");
    }
    size_t* rc = malloc(sizeof(size_t));
    *rc = 1;
    SharedFileStreamCursorInfo* info = malloc(sizeof(*info));
    *info = (typeof(*info)) { pos, false, false };
    return (SharedFileStream) { f, info, rc };
}

void shared_file_stream_free(SharedFileStream* s) {
    free(s->info);
    s->info = NULL;
    (*s->rc)--;
    if (*s->rc == 0) {
        free(s->rc);
        s->rc = NULL;
        fclose(s->file);
    }
}

SharedFileStream shared_file_stream_clone(SharedFileStream* s) {
    typeof(s->info) info_clone = malloc(sizeof(*info_clone));
    *info_clone = *s->info;
    (*s->rc)++;
    return (SharedFileStream) { s->file, info_clone, s->rc };
}

size_t shared_file_stream_read(SharedFileStream* s, UInt8Slice buf) {
start:
    if (buf.len == 0) return 0;
    FILE* const f = s->file;
    typeof(s->info) i = s->info;
    if (i->eof_reached) return 0;
    if (i->had_err) {
        PANIC(
            "something went wrong - file stream with pointer %p had it's error "
            "flag set",
            (void*)f
        );
    }
    // I guess this isn't thread safe...
    if (fsetpos(f, &i->pos)) { // error
        PANIC("i have no idea how to handle this");
    }
    clearerr(f);
    size_t n_read = fread(buf.ptr, 1, buf.len, f);
    i->eof_reached = feof(f);
    i->had_err = ferror(f);
    if (fsetpos(f, &i->pos)) PANIC("i have no idea how to handle this");
    if (n_read == 0) goto start;
    return n_read;
}

// ------ Shared Buffered File Stream ------------------------------------------

#define S SharedBufferedFileStream
#define SBox SharedBufferedFileStreamBox

void shared_buffered_file_stream_drop_prefix(SBox* box) {
    // how much to drop?
    size_t n_drop = 0;
    while (*array_get(&box->marks, n_drop) == 0 && n_drop < box->marks.len - 1)
        n_drop++;
    // drop!
    if (n_drop == 0) return;
    array_pop_n_at(&box->buf, 0, n_drop);
    array_pop_n_at(&box->marks, 0, n_drop);
    array_trim(&box->buf);
    array_trim(&box->marks);
    box->bytes_dropped += n_drop;
}

void shared_buffered_file_stream_free_if_done(S* s) {
    if (s->box->marks.len == 1 && *array_first(&s->box->marks) == 0) {
        array_free(&s->box->buf);
        array_free(&s->box->marks);
        free(s->box);
        fclose(s->file);
        s->box = NULL;
        s->file = NULL;
    }
}

S shared_buffered_file_stream_from_file_stream(FileStream s) {
    SBox* box = malloc(sizeof(SBox));
    *box = (SBox) { array_empty(), array_empty(), 0 };
    array_push(&box->marks, 1);
    return (S) { s.file, box, 0 };
}

uint16_t* shared_buffered_file_stream_mark(S* s) {
    return array_get(&s->box->marks, s->box->bytes_dropped + s->index);
}

void shared_buffered_file_stream_free(S* s) {
    *shared_buffered_file_stream_mark(s) -= 1;
    shared_buffered_file_stream_drop_prefix(s->box);
    shared_buffered_file_stream_free_if_done(s);
}

S shared_buffered_file_stream_clone(S* s) {
    *shared_buffered_file_stream_mark(s) += 1;
    return (S) { s->file, s->box, s->index };
}

size_t shared_buffered_file_stream_read(S* s, UInt8Slice slice) {
    SBox* b = s->box;
    if (s->index == b->marks.len - 1) {
    again:
        // read
        if (feof(s->file)) return 0;
        if (ferror(s->file)) PANIC("not sure what to do");
        size_t n_read = fread(slice.ptr, 1, slice.len, s->file);
        if (n_read == 0) goto again;
        // update
        for (size_t i = 0; i < n_read; i++) {
            array_push(&s->box->buf, *slice_get(&slice, i));
            array_push(&s->box->marks, 0);
        }
        *shared_buffered_file_stream_mark(s) -= 1;
        s->index += n_read;
        *shared_buffered_file_stream_mark(s) += 1;
        shared_buffered_file_stream_drop_prefix(s->box);
        return n_read;
    } else {
        // read
        size_t real_index = s->index - s->box->bytes_dropped;
        size_t n_left = s->box->buf.len - real_index;
        size_t n = MIN(slice.len, n_left);
        memcpy(slice.ptr, s->box->buf.ptr + real_index, n);
        // update
        *shared_buffered_file_stream_mark(s) -= 1;
        s->index += n;
        *shared_buffered_file_stream_mark(s) += 1;
        shared_buffered_file_stream_drop_prefix(s->box);
        return n;
    }
}

#undef S
#undef SBox

// ------ Conversions ----------------------------------------------------------

#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    Stream stream_from_##SNAKE##_stream(PASCAL##Stream s) {                    \
        return (Stream) { STREAM_KIND_##UPPER, { .SNAKE = s } };               \
    }
X_ON_STREAM_KINDS
#undef X
