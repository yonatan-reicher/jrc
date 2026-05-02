#include "slice.h"

CharSlice slice_of_str(char* str) {
    return (CharSlice) { .ptr = str, .len = strlen(str) };
}

ConstCharSlice slice_of_const_str(const char* str) {
    return (ConstCharSlice) { .ptr = str, .len = strlen(str) };
}
