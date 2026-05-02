#include "lexer.h"
#include <ctype.h>

Lexer lexer_new(const char* text) {
    return (Lexer) {
        .text = text,
        .pos = text_pos_start(),
    };
}

void lexer_free(Lexer* lexer) {
    (void)lexer;
}

bool lexer_is_done(const Lexer* lexer) {
    return lexer->text[lexer->pos.index] == '\0';
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

size_t token_len(const Token* t) {
    return t->span.end.index - t->span.start.index;
}

char peek(const Lexer* lexer) {
    return lexer->text[lexer->pos.index];
}

void advance_right(Lexer* lexer) {
    lexer->pos.index++;
    lexer->pos.col++;
}

void advance_down(Lexer* lexer) {
    lexer->pos.index++;
    lexer->pos.row++;
    lexer->pos.col = 1;
}

char pop(Lexer* l) {
    char c = peek(l);
    if (c == '\n') advance_down(l);
    else advance_right(l);
    return c;
}

typedef struct TokenStart {
    Token t;
} TokenStart;

TokenStart start_token(const Lexer* l) {
    return (TokenStart) {
        .t = {
            .kind = -1, // To be filled!
            .text = l->text + l->pos.index,
            .span = {
                .start = l->pos,
                .end = {}, // To be filled!
            },
        },
    };
}

Token end_token(const Lexer* l, TokenStart token_start, TokenKind kind) {
    Token token = token_start.t;
    token.span.end = l->pos;
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
