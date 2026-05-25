#pragma once
#include "text_pos.h"

typedef enum TokenKind {
    TOKEN_KIND_NULL,
    TOKEN_KIND_ERROR,
    TOKEN_KIND_EOF,
    /// 123
    TOKEN_KIND_INT,
    /// hello
    TOKEN_KIND_WORD,
    /// "world"
    TOKEN_KIND_STR,
    /// '+'
    TOKEN_KIND_PLUS,
    /// '-'
    TOKEN_KIND_MINUS,
    /// '*'
    TOKEN_KIND_STAR,
    /// '/'
    TOKEN_KIND_SLASH,
    /// '%'
    TOKEN_KIND_PERCENT,
    /// '('
    TOKEN_KIND_LPAREN,
    /// ')'
    TOKEN_KIND_RPAREN,
    /// ':='
    TOKEN_KIND_COLON_EQ,
    /// ':'
    TOKEN_KIND_COLON,
    /// ';'
    TOKEN_KIND_SEMICOLON,
    /// '['
    TOKEN_KIND_LSQUARE,
    /// ']'
    TOKEN_KIND_RSQUARE,
    /// '{'
    TOKEN_KIND_LCURLY,
    /// '}'
    TOKEN_KIND_RCURLY,
    /// '->'
    TOKEN_KIND_THIN_ARROW,
    /// '=>'
    TOKEN_KIND_FAT_ARROW,
    /// '='
    TOKEN_KIND_EQ,
    /// '!'
    TOKEN_KIND_BANG,
    /// '!='
    TOKEN_KIND_BANG_EQ,
} TokenKind;

typedef struct Token {
    TokenKind kind;
    /// A pointer to this token's text buffer. This just points to the original
    /// source text, offset by the span's start, but this may change in the
    /// future.
    /// Not owned by the token.
    const char* text;
    TextSpan span;
} Token;

#define TOKEN_PRINTF_FORMAT "Token{kind: %s, span: " TEXT_SPAN_PRINTF_FORMAT "}"

#define TOKEN_PRINTF(T) token_kind_name((T).kind), TEXT_SPAN_PRINTF((T).span)

const char* token_kind_name(TokenKind kind);

size_t token_len(const Token* t);

struct ConstCharSlice token_slice(const Token* t);
