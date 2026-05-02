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
Token lexer_get_str(Lexer*);
TokenKind lex_single_char_symbol(char c);

Token lexer_pop(Lexer* lexer) {
    lexer_skip_whitespace(lexer);

    if (lexer_is_done(lexer)) {
        return end_token(lexer, start_token(lexer), TOKEN_KIND_EOF);
    }

    const char c = peek(lexer);
    if (lex_single_char_symbol(c) != TOKEN_KIND_NULL) {
        const TokenKind kind = lex_single_char_symbol(c);
        const TokenStart start = start_token(lexer);
        pop(lexer);
        return end_token(lexer, start, kind);
    }
    if (isdigit(c)) return lexer_get_int(lexer);
    if (isalpha(c)) return lexer_get_word(lexer);
    if (c == '"') return lexer_get_str(lexer);

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

Token lexer_get_str(Lexer* lexer) {
    TokenStart t = start_token(lexer);
    pop(lexer); // Skip the opening quote.
    while (peek(lexer) != '"') {
        if (lexer_is_done(lexer)) { // Unterminated string.
            return end_token(lexer, t, TOKEN_KIND_ERROR);
        }
        pop(lexer);
    }
    pop(lexer); // Skip the closing quote.
    return end_token(lexer, t, TOKEN_KIND_STR);
}

TokenKind lex_single_char_symbol(char c) {
    switch (c) {
        case '+': return TOKEN_KIND_PLUS;
        case '-': return TOKEN_KIND_MINUS;
        case '*': return TOKEN_KIND_STAR;
        case '/': return TOKEN_KIND_SLASH;
        case '(': return TOKEN_KIND_LPAREN;
        case ')': return TOKEN_KIND_RPAREN;
        default: return TOKEN_KIND_NULL; // Not a recognized symbol.
    }
}
