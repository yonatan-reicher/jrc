#include "slice.h"
#include "basic.h"

int main(void) {
#define MY_STR "Hello world"
    Slice(char) s = slice_of_str(MY_STR);
    printf("s: %.*s\n", (int)s.len, s.ptr);
    return 0;
}
