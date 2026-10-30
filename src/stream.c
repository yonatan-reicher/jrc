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

// ------ Conversions ----------------------------------------------------------

#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    Stream stream_from_##SNAKE##_stream(PASCAL##Stream s) {                    \
        return (Stream) { STREAM_KIND_##UPPER, { .SNAKE = s } };               \
    }
X_ON_STREAM_KINDS
#undef X
