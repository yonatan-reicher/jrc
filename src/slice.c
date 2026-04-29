#include "slice.h"

Slice(char) slice_of_str(char *str) {
    size_t len = strlen(str);
    return slice(char, len, str);
}
