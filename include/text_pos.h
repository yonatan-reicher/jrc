#pragma once
#include <inttypes.h>
#include <stddef.h>

// =============================================================================
//                                     Types
// =============================================================================

typedef uint32_t TextRow, TextCol, TextLen;

typedef struct TextPos {
    size_t index;
    TextRow row;
    TextCol col;
} TextPos;

typedef struct TextSpan {
    TextPos start;
    TextPos end;
} TextSpan;

// =============================================================================
//                                    Macros
// =============================================================================

#define TEXT_POS_PRINTF_FORMAT "[%" PRIu32 ":%" PRIu32 "]"
#define TEXT_POS_PRINTF(P) (P).row, (P).col

#define TEXT_SPAN_PRINTF_FORMAT "[%" PRIu32 ":%" PRIu32 "-%" PRIu32 ":%" PRIu32 "]"
#define TEXT_SPAN_PRINTF(S) (S).start.row, (S).start.col, (S).end.row, (S).end.col

// =============================================================================
//                                   Functions
// =============================================================================

TextPos text_pos_start(void);
