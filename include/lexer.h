#pragma once
#include "text_pos.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum TokenKind {
    TOKEN_KIND_NULL,
    TOKEN_KIND_ERROR,
    TOKEN_KIND_EOF,
    TOKEN_KIND_INT,
    TOKEN_KIND_WORD,
} TokenKind;

typedef struct Token {
    TokenKind kind;
    TextSpan span;
} Token;

#define TOKEN_PRINTF_FORMAT "Token{kind: %s, span: " TEXT_SPAN_PRINTF_FORMAT "}"

#define TOKEN_PRINTF(T) token_kind_name((T).kind), TEXT_SPAN_PRINTF((T).span)

typedef struct Lexer {
    const char* text;
    TextPos pos;
} Lexer;

Lexer lexer_new(const char* text);

void lexer_free(Lexer*);

bool lexer_is_done(const Lexer*);

Token lexer_pop(Lexer*);

const char* token_kind_name(TokenKind kind);
