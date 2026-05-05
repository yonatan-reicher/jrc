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
static void lexer_skip_whitespace(Lexer*);
static Token lexer_get_int(Lexer*);
static Token lexer_get_word(Lexer*);
static Token lexer_get_str(Lexer*);
static Token lexer_get_sym(Lexer*);

Token lexer_pop(Lexer* lexer) {
    lexer_skip_whitespace(lexer);

    // Check EOF
    if (lexer_is_done(lexer)) {
        return end_token(lexer, start_token(lexer), TOKEN_KIND_EOF);
    }

    // Check symbols first
    const Token sym_token = lexer_get_sym(lexer);
    if (sym_token.kind != TOKEN_KIND_NULL) {
        return sym_token;
    }

    // Rest of the tokens
    const char c = peek(lexer);
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

static TokenKind lexer_get_sym_kind(Lexer* l) {
    switch (peek(l)) {
        case '+': pop(l); return TOKEN_KIND_PLUS;
        case '-': pop(l); return TOKEN_KIND_MINUS;
        case '*': pop(l); return TOKEN_KIND_STAR;
        case '/': pop(l); return TOKEN_KIND_SLASH;
        case '(': pop(l); return TOKEN_KIND_LPAREN;
        case ')': pop(l); return TOKEN_KIND_RPAREN;
        case '[': pop(l); return TOKEN_KIND_LSQUARE;
        case ']': pop(l); return TOKEN_KIND_RSQUARE;
        case '{': pop(l); return TOKEN_KIND_LCURLY;
        case '}': pop(l); return TOKEN_KIND_RCURLY;
        case ';': pop(l); return TOKEN_KIND_SEMICOLON;
        case ':':
            pop(l);
            return peek(l) == '=' ? (pop(l), TOKEN_KIND_COLON_EQ)
                                  : TOKEN_KIND_COLON;
        default: return TOKEN_KIND_NULL; // Not a recognized symbol.
    }
}

static Token lexer_get_sym(Lexer* l) {
    TokenStart t = start_token(l);
    const TokenKind kind = lexer_get_sym_kind(l);
    return end_token(l, t, kind);
}
