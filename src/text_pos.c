#include "text_pos.h"

TextPos text_pos_start(void) {
    return (TextPos) {
        .index = 0,
        .row = 1,
        .col = 1,
    };
}
