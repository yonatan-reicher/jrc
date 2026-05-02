#pragma once
#include "text_pos.h"
#include "token.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct Lexer {
    const char* text;
    TextPos pos;
} Lexer;

Lexer lexer_new(const char* text);

void lexer_free(Lexer*);

bool lexer_is_done(const Lexer*);

Token lexer_pop(Lexer*);
