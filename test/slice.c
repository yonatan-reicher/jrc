#include "slice.h"
#include "basic.h"

void test_slice_empty(void) {
    CharSlice s = slice_empty();
    EXPECT(s.ptr == NULL, "Expected empty slice to have NULL pointer");
    EXPECT(s.len == 0, "Expected empty slice to have length 0");
}

int main(void) {
    test_slice_empty();
    return 0;
}
