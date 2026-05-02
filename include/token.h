#pragma once
#include "text_pos.h"

typedef enum TokenKind {
    TOKEN_KIND_NULL,
    TOKEN_KIND_ERROR,
    TOKEN_KIND_EOF,
    TOKEN_KIND_INT,
    TOKEN_KIND_WORD,
} TokenKind;

typedef struct Token {
    TokenKind kind;
    const char* text;
    TextSpan span;
} Token;

#define TOKEN_PRINTF_FORMAT "Token{kind: %s, span: " TEXT_SPAN_PRINTF_FORMAT "}"

#define TOKEN_PRINTF(T) token_kind_name((T).kind), TEXT_SPAN_PRINTF((T).span)

const char* token_kind_name(TokenKind kind);

size_t token_len(const Token* t);
