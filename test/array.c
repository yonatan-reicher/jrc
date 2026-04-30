#include "array.h"
#include "basic.h"

DEFINE_ARRAY(char);

int main(void) {
    Array(char) chars = array_empty(char);
    Array(char) chars1 = array_empty(char);
    if (chars.len != chars1.len) PANIC("aaa");
    return 0;
}
