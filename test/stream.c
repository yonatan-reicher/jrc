#include "stream.h"
#include "basic.h"

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

int main(void) {
    test_empty();
    test_slice_stream_read_exact();
    test_slice_stream_read_not_enough();
    test_slice_stream_read_too_much();
    return 0;
}
