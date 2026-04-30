#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

typedef enum TokenKind {
    TOKEN_KIND_NULL,
    TOKEN_KIND_ERROR,
    TOKEN_KIND_EOF,
    TOKEN_KIND_INT,
    TOKEN_KIND_WORD,
} TokenKind;

typedef size_t LexerIndex;
typedef uint16_t LexerRow, LexerCol, TokenLen;

typedef struct Token {
    LexerIndex index;
    LexerRow row;
    LexerCol col;
    TokenLen len;
    TokenKind kind;
} Token;

#define TOKEN_PRINTF_FORMAT \
    "Token{"                \
    "index: %zu, "          \
    "row: %" PRIu16 ", "     \
    "col: %" PRIu16 ", "     \
    "len: %" PRIu16 ", "     \
    "kind: %s"              \
    "}"

#define TOKEN_PRINTF(T) \
    (T).index, (T).row, (T).col, (T).len, token_kind_name((T).kind)

typedef struct Lexer {
    const char* text;
    LexerIndex index;
    LexerRow row;
    LexerCol col;
} Lexer;

Lexer lexer_new(const char* text);

void lexer_free(Lexer*);

bool lexer_is_done(const Lexer*);

Token lexer_pop(Lexer*);

const char* token_kind_name(TokenKind kind);
