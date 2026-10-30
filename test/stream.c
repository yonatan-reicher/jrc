#include "stream.h"
#include "basic.h"
#include "slice.h"
#include "str.h"

void test_empty(void) {
    Stream s = stream_empty();
    uint8_t buf[1024];
    EXPECT(
        stream_read(&s, (UInt8Slice)slice_new(buf, 1024)) == 0,
        "empty stream should not read"
    );
}

void test_slice_stream_read_exact(void) {
#define N 1024
    // input
    uint8_t inp_buf[N];
    for (size_t i = 0; i < N; i++) inp_buf[i] = (uint8_t)i;
    // output
    Stream s = stream_of_slice((ConstUInt8Slice)slice_new(inp_buf, N));
    uint8_t out_buf[N];
    size_t n_read = stream_read(&s, (UInt8Slice)slice_new(out_buf, N));
    // expect
    EXPECT(n_read == N, "the buffer has enough space to read everything");
    for (size_t i = 0; i < N; i++)
        EXPECT(
            out_buf[i] == (uint8_t)i,
            "should have been transferred (iter %zu)",
            i
        );
    n_read = stream_read(&s, (UInt8Slice)slice_new(out_buf, N));
    EXPECT(n_read == 0, "empty");
#undef N
}

void test_slice_stream_read_not_enough(void) {
#define N 1024
    // input
    uint8_t inp_buf[N];
    for (size_t i = 0; i < N; i++) inp_buf[i] = (uint8_t)i;
    // output
    Stream s = stream_of_slice((ConstUInt8Slice)slice_new(inp_buf, N));
    uint8_t out_buf[2 * N];
    size_t n_read = stream_read(&s, (UInt8Slice)slice_new(out_buf, 2 * N));
    // expect
    EXPECT(n_read == N, "the buffer has more than enough space");
    for (size_t i = 0; i < N; i++)
        EXPECT(
            out_buf[i] == (uint8_t)i,
            "should have been transferred (iter %zu)",
            i
        );
    n_read = stream_read(&s, (UInt8Slice)slice_new(out_buf, 2 * N));
    EXPECT(n_read == 0, "empty");
#undef N
}

void test_slice_stream_read_too_much(void) {
#define N 1024
    // input
    uint8_t inp_buf[3 * N];
    for (size_t i = 0; i < 3 * N; i++) inp_buf[i] = (uint8_t)i;
    // output
    Stream s = stream_of_slice((ConstUInt8Slice)slice_new(inp_buf, 3 * N));
    uint8_t out_buf[2 * N];
    size_t n_read = stream_read(&s, (UInt8Slice)slice_new(out_buf, 2 * N));
    // expect
    EXPECT(n_read == 2 * N, "the buffer has less than enough space");
    for (size_t i = 0; i < 2 * N; i++)
        EXPECT(
            out_buf[i] == (uint8_t)i,
            "should have been transferred (iter %zu)",
            i
        );
    // clear....
    for (size_t i = 0; i < 2 * N; i++) out_buf[i] = 0;
    // expect again
    n_read = stream_read(&s, (UInt8Slice)slice_new(out_buf, 2 * N));
    EXPECT(n_read == N, "only this much is left");
    for (size_t i = 0; i < N; i++)
        EXPECT(
            out_buf[i] == (uint8_t)i,
            "should have been transferred (iter %zu)",
            i
        );
    n_read = stream_read(&s, (UInt8Slice)slice_new(out_buf, 2 * N));
    EXPECT(n_read == 0, "empty");
#undef N
}

void test_sizeof_stream(void) {
    EXPECT(
        sizeof(Stream) <= sizeof(size_t) * 4,
        "struct Stream is bigger than expected"
    );
}

void test_str_stream_read(void) {
    Stream s = stream_of_str("hi");
    uint8_t buf[1024] = "aaaaaaaaaaa";
    size_t n_read = stream_read(&s, (UInt8Slice)slice_new(buf, ARRAY_LEN(buf)));
    EXPECT(n_read == 2, "should read two letters");
    EXPECT(buf[0] == 'h', "read first letter");
    EXPECT(buf[1] == 'i', "read second letter");
    EXPECT(buf[2] == 'a', "make sure that '\\0' was not written");
    stream_free(&s);
}

void test_from_str_stream_read(void) {
    Stream s = stream_from_str(str_clone("hi"));
    uint8_t buf[1024] = "aaaaaaaaaaa";
    size_t n_read = stream_read(&s, (UInt8Slice)slice_new(buf, ARRAY_LEN(buf)));
    EXPECT(n_read == 2, "should read two letters");
    EXPECT(buf[0] == 'h', "read first letter");
    EXPECT(buf[1] == 'i', "read second letter");
    EXPECT(buf[2] == 'a', "make sure that '\\0' was not written");
    stream_free(&s);
}

void test_stream_read_to_array(void) {
    char str[1024 * 5 + 1] = { 0 };
    for (size_t i = 0; i < ARRAY_LEN(str) - 1; i++) str[i] = 'a';
    str[ARRAY_LEN(str) - 1] = 0;
    Stream s = stream_of_str(str);
    UInt8Array a = stream_read_to_array(&s);
    EXPECT(
        a.len == ARRAY_LEN(str) - 1,
        "expect the length to be '%zu' but was actually '%zu'",
        ARRAY_LEN(str) - 1,
        a.len
    );
    array_free(&a);
    stream_free(&s);
}

void test_file_stream_clone(void) {
    FILE* f = tmpfile();
    fputs("Hello woorld!", f);
    rewind(f);
    Stream s1 = stream_from_file(f);
    Stream s2;
    uint8_t buf[1024];
    UInt8Slice slice = slice_of_fixed_array(buf);
    size_t n_read = stream_read(&s1, slice_take(slice, 6));
    EXPECT(n_read == 6, "should read");
    s2 = stream_clone(&s1);
    // Now read the rest - twice.
    n_read = stream_read(&s1, slice);
    EXPECT(n_read == 7, "7 characters left");
    n_read = stream_read(&s1, slice);
    EXPECT(n_read == 0, "end");
    n_read = stream_read(&s2, slice);
    EXPECT(n_read == 7, "7 characters left");
    n_read = stream_read(&s2, slice);
    EXPECT(n_read == 0, "end");
    stream_free(&s1);
    stream_free(&s2);
}

int main(void) {
    test_empty();
    test_slice_stream_read_exact();
    test_slice_stream_read_not_enough();
    test_slice_stream_read_too_much();
    test_str_stream_read();
    test_from_str_stream_read();
    test_sizeof_stream();
    test_stream_read_to_array();
    test_file_stream_clone();
    return 0;
}
