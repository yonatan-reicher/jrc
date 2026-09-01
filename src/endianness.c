#include "endianness.h"

Endianness endianness_get() {
    // Check endianness by, ironically, checking the start of an integer.
    int x = 1;
    uint8_t* p = (uint8_t*)&x;
    // If the least significant byte is at the lower memory address, we are
    // little endian-ing.
    return *p == 1 ? ENDIANNESS_LIL : ENDIANNESS_BIG;
}

#define E endianness_get
#define BIG ENDIANNESS_BIG
#define LIL ENDIANNESS_LIL

bool endianness_is_big() {
    return E() == BIG;
}

bool endianness_is_lil() {
    return E() == LIL;
}

static void swap(uint8_t* a, uint8_t* b) {
    uint8_t c = *a;
    *a = *b;
    *b = c;
}

void endianness_reverse(UInt8Slice s) {
    size_t j = s.len;
    for (size_t i = 0; 2 * i < s.len; i++) {
        j--;
        swap(slice_get(&s, i), slice_get(&s, j));
    }
}

#define REV endianness_reverse

void endianness_to(UInt8Slice s, Endianness e) {
    if (endianness_get() != e) REV(s);
}

void endianness_of(UInt8Slice s, Endianness e) {
    // Turns out this is the same operation... No shit.
    if (endianness_get() != e) REV(s);
}

void endianness_to_big(UInt8Slice s) {
    endianness_to(s, BIG);
}
void endianness_to_lil(UInt8Slice s) {
    endianness_to(s, LIL);
}

void endianness_of_big(UInt8Slice s) {
    endianness_of(s, BIG);
}
void endianness_of_lil(UInt8Slice s) {
    endianness_of(s, LIL);
}
