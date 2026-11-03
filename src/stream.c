#include "stream.h"
#include "slice.h"
#include "str.h"
#include <stdatomic.h>

// ------ Stream implementation ------------------------------------------------

// Abstract Functions

void stream_free(Stream* s) {
    switch (s->kind) {
#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    case STREAM_KIND_##UPPER: SNAKE##_stream_free(&s->data.SNAKE); break;
        X_ON_STREAM_KINDS
#undef X
    }
    *s = stream_empty();
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

bool stream_cmd_inp_out_err(
    const char* name,
    ConstCharPtrSlice args,
    Stream inp,
    Stream* out,
    Stream* err
) {
    CmdStream* out_inner = out ? &out->data.cmd : NULL;
    CmdStream* err_inner = err ? &err->data.cmd : NULL;
    bool success = cmd_stream(name, args, inp, out_inner, err_inner);
    if (success) {
        if (out) out->kind = STREAM_KIND_CMD;
        if (err) err->kind = STREAM_KIND_CMD;
    }
    return success;
}

Stream stream_cmd_out(const char* name, ConstCharPtrSlice args) {
    return stream_cmd_inp_out(name, args, stream_empty());
}

Stream stream_cmd_inp_out(
    const char* name, ConstCharPtrSlice args, Stream inp
) {
    Stream ret;
    EXPECT_ERRNO(stream_cmd_inp_out_err(name, args, inp, &ret, NULL));
    return ret;
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

/// NOTE: this is totally not thread safe
static bool file_supports_set_pos(FILE* f) {
    fpos_t p;
    // Make sure we can both get and set the position.
    if (fgetpos(f, &p) == -1) return false;
    // Only try setting the position if we actually could get it!
    if (fsetpos(f, &p) == -1) return false;
    return true;
}

Stream file_stream_clone(StreamKind* kind, FileStream* s) {
    if (file_supports_set_pos(s->file)) {
        SharedFileStream* ss = (SharedFileStream*)s;
        *kind = STREAM_KIND_SHARED_FILE;
        *ss = shared_file_stream_from_file_stream(*s);
        return stream_from_shared_file_stream(shared_file_stream_clone(ss));
    } else {
        BufferedStream* ss = (BufferedStream*)s;
        *kind = STREAM_KIND_BUFFERED;
        *ss = buffered_stream_from_stream(*(Stream*)s);
        return stream_from_buffered_stream(buffered_stream_clone(ss));
    }
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

// ------ Buffered Stream ------------------------------------------------------

/// Implementation:
/// All information is stored in a *box*. The box is mutable and shared between
/// all clones of this stream. It contains:
/// - data buffer
/// - mark buffer
/// - amount of bytes already dropped
///
/// Mark buffer: for each byte in the data buffer, has a counter counting the
/// amount of streams that point to that byte as the next one to read. Also
/// holds one more counter at the end, counting the amount of streams that
/// point at unread bytes.
///
/// TODO: mark by chunks instead of singular bytes.
typedef struct BufferedStreamBox {
    UInt8Array buf;
    UInt16Array marks;
    /// When all the streams read some prefix of the buffer, that prefix is
    /// dropped, and this starting offset is updated to reflect it.
    size_t bytes_dropped;
    Stream inner;
} BufferedStreamBox;

void buffered_stream_drop_prefix(BufferedStreamBox* box) {
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

void buffered_stream_free_if_done(BufferedStream* s) {
    if (s->box->marks.len == 1 && *array_first(&s->box->marks) == 0) {
        array_free(&s->box->buf);
        array_free(&s->box->marks);
        stream_free(&s->box->inner);
        free(s->box);
        memset(s->box, 0, sizeof(BufferedStreamBox));
        s->box = NULL;
    }
}

BufferedStream buffered_stream_from_stream(Stream s) {
    BufferedStreamBox* box = malloc(sizeof(BufferedStreamBox));
    *box = (BufferedStreamBox) { array_empty(), array_empty(), 0, s };
    array_push(&box->marks, 1);
    return (BufferedStream) { 0, box };
}

uint16_t* buffered_stream_mark(BufferedStream* s) {
    return array_get(&s->box->marks, s->box->bytes_dropped + s->index);
}

void buffered_stream_free(BufferedStream* s) {
    *buffered_stream_mark(s) -= 1;
    buffered_stream_drop_prefix(s->box);
    buffered_stream_free_if_done(s);
}

BufferedStream buffered_stream_clone(BufferedStream* s) {
    *buffered_stream_mark(s) += 1;
    return *s;
}

size_t buffered_stream_read(BufferedStream* s, UInt8Slice slice) {
    BufferedStreamBox* b = s->box;
    if (s->index == b->marks.len - 1) {
        // read
        size_t n_read = stream_read(&b->inner, slice);
        if (n_read == 0) return 0;
        // update
        for (size_t i = 0; i < n_read; i++) {
            array_push(&s->box->buf, *slice_get(&slice, i));
            array_push(&s->box->marks, 0);
        }
        *buffered_stream_mark(s) -= 1;
        s->index += n_read;
        *buffered_stream_mark(s) += 1;
        buffered_stream_drop_prefix(s->box);
        return n_read;
    } else {
        // read
        size_t real_index = s->index - s->box->bytes_dropped;
        size_t n_left = s->box->buf.len - real_index;
        size_t n = MIN(slice.len, n_left);
        memcpy(slice.ptr, s->box->buf.ptr + real_index, n);
        // update
        *buffered_stream_mark(s) -= 1;
        s->index += n;
        *buffered_stream_mark(s) += 1;
        buffered_stream_drop_prefix(s->box);
        return n;
    }
}

// ------ Command Stream -------------------------------------------------------

#if defined(unix) || defined(__unix) || defined(__unix__) ||                   \
    (defined(__APPLE__) && defined(__MACH__))
#include <signal.h>
#include <unistd.h>
#ifdef _POSIX_THREADS
#define HAS_PTHREAD
#endif
#endif

#ifdef HAS_PTHREAD
#include <pthread.h>
#elifdef _WIN32
#error "not implemented yet"
#endif

/// This is possible amount of references to the Command Stream's data, which is
/// shared between both an output stream and an error stream.
typedef uint8_t CmdStreamDataRc;

/// Shared data between an output command stream and an error command stream.
typedef struct CmdStreamData {
    /// A stream reading standard output of the command.
    FileStream out_stream;
    /// A stream reading standard error of the command.
    FileStream err_stream;
    /// Handle to the thread that feeds the command input.
    pthread_t feeder_thread;
    struct FeederThreadArgs* feeder_args;
    /// Handle to the process that runs the command.
    pid_t cmd_process;
    /// Is this data referenced by one, or two streams? This is decremented when
    /// one stream is freed.
    CmdStreamDataRc rc;
} CmdStreamData;

typedef struct FeederThreadArgs {
    Stream inp;
    int write_fd;
    atomic_bool please_close;
} FeederThreadArgs;

static void* feeder_thread(void* _args) {
    FeederThreadArgs* args = (FeederThreadArgs*)_args;
    STREAM_READ_BY_CHUNKS(&args->inp, 1024, chunk) {
        size_t n_left = chunk.len;
        while (n_left > 0) {
            if (atomic_load(&args->please_close)) goto end;
            ssize_t n_wrote = write(args->write_fd, chunk.ptr, chunk.len);
            EXPECT_ERRNO(n_wrote != -1);
            EXPECT(n_wrote > 0, "must write more than zero bytes");
            n_left -= (size_t)n_wrote;
        }
    }
end:
    return NULL;
}

static void cmd_stream_data_free(CmdStreamData* d) {
    SYS(kill(d->cmd_process, SIGKILL));
    atomic_store(&d->feeder_args->please_close, true);
    EXPECT_PTHREAD(pthread_join(d->feeder_thread, NULL));
    file_stream_free(&d->out_stream);
    file_stream_free(&d->err_stream);
    stream_free(&d->feeder_args->inp);
    free(d->feeder_args);
    free(d);
}

bool cmd_stream(
    const char* name,
    ConstCharPtrSlice args,
    Stream inp,
    CmdStream* out,
    CmdStream* err
) {
    // create pipes
    int inp_pipe[2], out_pipe[2], err_pipe[2];
    SYS(pipe(inp_pipe));
    SYS(pipe(out_pipe));
    SYS(pipe(err_pipe));
    int inp_read_fd = inp_pipe[0], inp_write_fd = inp_pipe[1];
    int out_read_fd = out_pipe[0], out_write_fd = out_pipe[1];
    int err_read_fd = err_pipe[0], err_write_fd = err_pipe[1];
    // create child thread
    pthread_t feeder_id;
    FeederThreadArgs* feeder_args = malloc(sizeof(FeederThreadArgs));
    *feeder_args = (FeederThreadArgs) { inp, inp_write_fd, false };
    EXPECT_PTHREAD(
        pthread_create(&feeder_id, NULL, feeder_thread, feeder_args)
    );
    // create a subprocess
    pid_t pid = fork();
    EXPECT_ERRNO(pid != -1);
    if (pid == 0) { // child process
        close(inp_write_fd);
        close(out_read_fd);
        close(err_read_fd);
        CharPtrArray args_null_terminated = array_empty();
        array_reserve(&args_null_terminated, args.len + 2);
        array_push(&args_null_terminated, str_clone(name));
        SLICE_FOREACH(&args, arg) {
            array_push(&args_null_terminated, str_clone(*arg));
        }
        array_push(&args_null_terminated, NULL);
        dup2(inp_read_fd, STDIN_FILENO);
        dup2(out_write_fd, STDOUT_FILENO);
        dup2(err_write_fd, STDERR_FILENO);
        execvp(name, args_null_terminated.ptr);
        // if execution of this code hasn't stopped, error.
        bool execution_stopped = false;
        EXPECT_ERRNO(execution_stopped);
    }
    close(inp_read_fd);
    close(out_write_fd);
    close(err_write_fd);
    FILE* out_read_file = fdopen(out_read_fd, "r");
    FILE* err_read_file = fdopen(err_read_fd, "r");
    FileStream out_stream = file_stream(out_read_file);
    FileStream err_stream = file_stream(err_read_file);
    CmdStreamData* data = malloc(sizeof(*data));
    const CmdStreamDataRc rc = (out ? 1 : 0) + (err ? 1 : 0);
    *data = (typeof(*data)) {
        out_stream, err_stream, feeder_id, feeder_args, pid, rc,
    };
    if (out) *out = (CmdStream) { data, CMD_STREAM_OUT };
    if (err) *err = (CmdStream) { data, CMD_STREAM_ERR };
    if (!out && !err) {
        ASSERT(data->rc == 0, "because there are zero references");
        cmd_stream_data_free(data);
    };
    return true;
}

void cmd_stream_free(CmdStream* s) {
    s->data->rc -= 1;
    if (s->data->rc == 0) cmd_stream_data_free(s->data);
}

Stream cmd_stream_clone(StreamKind* kind, CmdStream*) {
    Stream* ss = (Stream*)(void*)kind;
    *ss = stream_from_buffered_stream(buffered_stream_from_stream(*ss));
    return stream_clone(ss);
}

FileStream* cmd_stream_file_stream(CmdStream* s) {
    switch (s->kind) {
        case CMD_STREAM_OUT: return &s->data->out_stream;
        case CMD_STREAM_ERR: return &s->data->err_stream;
    }
}

size_t cmd_stream_read(CmdStream* s, UInt8Slice buf) {
    FileStream* inner = cmd_stream_file_stream(s);
    return file_stream_read(inner, buf);
}

// ------ Conversions ----------------------------------------------------------

#define X(SNAKE, PASCAL, UPPER, CLONE_KIND)                                    \
    Stream stream_from_##SNAKE##_stream(PASCAL##Stream s) {                    \
        return (Stream) { STREAM_KIND_##UPPER, { .SNAKE = s } };               \
    }
X_ON_STREAM_KINDS
#undef X
