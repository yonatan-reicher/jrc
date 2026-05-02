#include "token.h"

const char* token_kind_name(TokenKind kind) {
    switch (kind) {
        case TOKEN_KIND_NULL: return "NULL";
        case TOKEN_KIND_ERROR: return "ERROR";
        case TOKEN_KIND_EOF: return "EOF";
        case TOKEN_KIND_INT: return "INT";
        case TOKEN_KIND_WORD: return "WORD";
        case TOKEN_KIND_STR: return "STR";
        case TOKEN_KIND_PLUS: return "PLUS";
        case TOKEN_KIND_MINUS: return "MINUS";
        case TOKEN_KIND_STAR: return "STAR";
        case TOKEN_KIND_SLASH: return "SLASH";
        case TOKEN_KIND_LPAREN: return "LPAREN";
        case TOKEN_KIND_RPAREN: return "RPAREN";
    }
}

size_t token_len(const Token* t) {
    return t->span.end.index - t->span.start.index;
}
