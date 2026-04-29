#include "../src/slice.h"
#include "../src/basic.h"

int main(void) {
#define MY_STR "Hello world"
    Slice(char) s = slice_of_str(MY_STR);
    return 0;
}
