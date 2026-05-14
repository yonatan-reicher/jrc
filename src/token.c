#include "token.h"
#include "slice.h"

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
        case TOKEN_KIND_PERCENT: return "PERCENT";
        case TOKEN_KIND_LPAREN: return "LPAREN";
        case TOKEN_KIND_RPAREN: return "RPAREN";
        case TOKEN_KIND_COLON: return "COLON";
        case TOKEN_KIND_SEMICOLON: return "SEMICOLON";
        case TOKEN_KIND_COLON_EQ: return "COLON-EQ";
        case TOKEN_KIND_LSQUARE: return "LSQUARE";
        case TOKEN_KIND_RSQUARE: return "RSQUARE";
        case TOKEN_KIND_LCURLY: return "LCURLY";
        case TOKEN_KIND_RCURLY: return "RCURLY";
    }
}

size_t token_len(const Token* t) {
    return t->span.end.index - t->span.start.index;
}

ConstCharSlice token_slice(const Token* t) {
    return (ConstCharSlice)slice(t->text, token_len(t));
}
