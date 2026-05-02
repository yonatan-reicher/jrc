#include "array.h"
#include "basic.h"

DECLARE_ARRAY(char, CharArray);

void test_empty_array(void) {
    CharArray arr = array_empty();
    EXPECT(arr.ptr == NULL, "Expected empty array to have NULL pointer");
    EXPECT(arr.len == 0, "Expected empty array to have length 0");
    EXPECT(arr.cap == 0, "Expected empty array to have capacity 0");
}

void test_pushing_and_popping(void) {
    CharArray arr = array_empty();
    for (char c = 'a'; c <= 'z'; c++) {
        array_push(&arr, c);
        EXPECT(
            arr.len == (size_t)(c - 'a' + 1),
            "Expected length to be correct after push"
        );
    }
    for (char c = 'z'; c >= 'a'; c--) {
        EXPECT(
            arr.ptr[arr.len - 1] == c,
            "Expected last element to be correct after push"
        );
        array_pop(&arr);
        EXPECT(
            arr.len == (size_t)(c - 'a'),
            "Expected length to be correct after pop"
        );
    }
    array_free(&arr);
    EXPECT(arr.ptr == NULL, "Expected array to be empty after free");
}

int main(void) {
    test_empty_array();
    test_pushing_and_popping();
    return 0;
}
