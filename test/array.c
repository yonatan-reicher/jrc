#include "array.h"
#include "basic.h"

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

static void test_reserve(void) {
    Int32Array a = array_empty();
    EXPECT(a.cap == 0, "capacity starts at 0");
    array_reserve(&a, 10);
    EXPECT(a.cap == 10, "reserve added place");
    array_reserve(&a, 10);
    EXPECT(a.cap == 10, "there already was enough");
    array_push(&a, 42);
    array_reserve(&a, 5);
    EXPECT(a.cap == 10, "there already was enough");
    array_reserve(&a, 10);
    EXPECT(a.cap == 11, "now there wasn't enough");
}

static void test_pop_n_at(void) {
    Int32Array arr = array_empty();
    for (int32_t i = 0; i < 6; i++) array_push(&arr, i);

    array_pop_n_at(&arr, 1, 2);
    EXPECT(arr.len == 4, "removing a range updates the length");
    EXPECT(arr.ptr[0] == 0 && arr.ptr[1] == 3 && arr.ptr[2] == 4 &&
               arr.ptr[3] == 5,
           "removing an interior range shifts the remaining elements");

    array_pop_n_at(&arr, 2, 2);
    EXPECT(arr.len == 2, "removing a suffix updates the length");
    EXPECT(arr.ptr[0] == 0 && arr.ptr[1] == 3,
           "removing a suffix preserves the preceding elements");

    array_pop_n_at(&arr, 0, 2);
    EXPECT(arr.len == 0, "removing the whole array is valid");
    array_free(&arr);
}

int main(void) {
    test_empty_array();
    test_pushing_and_popping();
    test_reserve();
    test_pop_n_at();
    return 0;
}
