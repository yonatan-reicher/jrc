#include "lexer.h"
#include <ctype.h>

typedef LexerIndex Index;
typedef LexerRow Row;
typedef LexerCol Col;
typedef TokenLen Len;

Lexer lexer_new(const char* text) {
    return (Lexer) {
        .text = text,
        .index = 0,
        .row = 1,
        .col = 1,
    };
}

void lexer_free(Lexer* lexer) {
    (void)lexer;
}

bool lexer_is_done(const Lexer* lexer) {
    return lexer->text[lexer->index] == '\0';
}

// =============================================================================
//                              Basic Functionality
// =============================================================================

const char* token_kind_name(TokenKind kind) {
    switch (kind) {
        case TOKEN_KIND_NULL: return "NULL";
        case TOKEN_KIND_ERROR: return "ERROR";
        case TOKEN_KIND_EOF: return "EOF";
        case TOKEN_KIND_INT: return "INT";
        case TOKEN_KIND_WORD: return "WORD";
        default: return "UNKNOWN";
    }
}

char peek(const Lexer* lexer) {
    return lexer->text[lexer->index];
}

void advanceRight(Lexer* lexer) {
    lexer->index++;
    lexer->col++;
}

void advanceDown(Lexer* lexer) {
    lexer->index++;
    lexer->row++;
    lexer->col = 1;
}

char pop(Lexer* l) {
    char c = peek(l);
    if (c == '\n') advanceDown(l);
    else advanceRight(l);
    return c;
}

typedef struct TokenStart {
    Token t;
} TokenStart;

TokenStart start_token(const Lexer* l) {
    return (TokenStart) {
        .t = (Token) {
            .index = l->index,
            .row = l->row,
            .col = l->col,
            .len = -1, // To be filled!
            .kind = -1, // To be filled!
        },
    };
}

Token end_token(const Lexer* l, TokenStart token_start, TokenKind kind) {
    Token token = token_start.t;
    token.len = l->index - token.index;
    token.kind = kind;
    return token;
}

// =============================================================================
//                              Main Functionality
// =============================================================================

// Declare some helpers.
void lexer_skip_whitespace(Lexer*);
Token lexer_get_int(Lexer*);
Token lexer_get_word(Lexer*);

Token lexer_pop(Lexer* lexer) {
    lexer_skip_whitespace(lexer);

    if (lexer_is_done(lexer)) {
        return end_token(lexer, start_token(lexer), TOKEN_KIND_EOF);
    }

    const char c = peek(lexer);
    if (isdigit(c)) return lexer_get_int(lexer);
    if (isalpha(c)) return lexer_get_word(lexer);

    return end_token(lexer, start_token(lexer), TOKEN_KIND_ERROR);
}

// Some helpers for `lexer_pop`:

void lexer_skip_whitespace(Lexer* lexer) {
    while (true) {
        if (lexer_is_done(lexer)) break;
        if (isspace(peek(lexer))) {
            pop(lexer);
            continue;
        }
        break;
    }
}

Token lexer_get_int(Lexer* lexer) {
    TokenStart t = start_token(lexer);
    while (isdigit(peek(lexer))) pop(lexer);
    return end_token(lexer, t, TOKEN_KIND_INT);
}

Token lexer_get_word(Lexer* lexer) {
    TokenStart t = start_token(lexer);
    while (isalnum(peek(lexer))) pop(lexer);
    return end_token(lexer, t, TOKEN_KIND_WORD);
}
