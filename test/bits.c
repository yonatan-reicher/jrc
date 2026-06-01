#include "bits.h"
#include "basic.h"

void test_first_bit(void) {
    EXPECT(FIRST_BIT(0b101100) == 0b100, "Expected first bit to be correct");
    EXPECT(FIRST_BIT(0b100000) == 0b100000, "Expected first bit to be correct");
    EXPECT(FIRST_BIT(0b111111) == 0b1, "Expected first bit to be correct");
}

void test_first_bit_zero(void) {
    EXPECT(FIRST_BIT(0) == 0, "Expected first bit of zero to be zero");
}

void test_all_but_first_bit(void) {
    EXPECT(ALL_BUT_FIRST_BIT(0b101100) == 0b101000, "Expected all but first bit to be correct");
    EXPECT(ALL_BUT_FIRST_BIT(0b100000) == 0, "Expected all but first bit to be correct");
    EXPECT(ALL_BUT_FIRST_BIT(0b111111) == 0b111110, "Expected all but first bit to be correct");
}

void test_all_but_first_bit_zero(void) {
    EXPECT(ALL_BUT_FIRST_BIT(0) == 0, "Expected all but first bit of zero to be zero");
}

void test_all_but_first_bit_minus_one(void) {
    EXPECT(ALL_BUT_FIRST_BIT(-1) == (-1 & ~1), "Expected all but first bit of -1 to be -2");
}

void test_is_exactly_one_bit(void) {
    EXPECT(IS_EXACTLY_ONE_BIT(0b1), "0b1 has exactly one bit");
    EXPECT(!IS_EXACTLY_ONE_BIT(0b0), "0b0 does not have any bits");
    EXPECT(!IS_EXACTLY_ONE_BIT(0b11), "0b11 has more than one bit");
    EXPECT(IS_EXACTLY_ONE_BIT(0b1000), "0b1000 has exactly one bit");
}

int main(void) {
    test_first_bit();
    test_first_bit_zero();
    test_all_but_first_bit();
    test_all_but_first_bit_zero();
    test_all_but_first_bit_minus_one();
    test_is_exactly_one_bit();
    return 0;
}
