#include "parser.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

const char* ast_kind_name(AstKind kind) {
    switch (kind) {
        case AST_NULL: return "NULL";
        case AST_ERROR: return "ERROR";
        case AST_INT: return "INT";
        case AST_VAR: return "VAR";
    }
}

Parser parser_new(Token (*get_token)(void* ctx), void* get_token_ctx) {
    Parser parser = {
        .get_token = get_token,
        .get_token_ctx = get_token_ctx,
        .arena = NULL,
        .arena_size = 0,
        .arena_capacity = 0,
        .previous_arenas = NULL,
        .n_previous_arenas = 0,
    };
    return parser;
}

void parser_free(Parser* p) {
    for (size_t i = 0; i < p->n_previous_arenas; i++) {
        free(p->previous_arenas[i]);
    }
    free(p->arena);
    *p = (Parser) { 0 };
}

void* alloc(Parser* p, size_t size) {
    // Allocate a new arena if the current one is done.
    if (p->arena_size + size > p->arena_capacity) {
        size_t new_capacity = (p->arena_size + size) * 2;
        void* new_arena = malloc(new_capacity);
        p->previous_arenas =
            realloc(p->previous_arenas, ++p->n_previous_arenas * sizeof(void*));
        p->previous_arenas[p->n_previous_arenas - 1] = p->arena;
        p->arena = new_arena;
        p->arena_capacity = new_capacity;
        p->arena_size = 0;
    }
    // Get the address of the new allocation.
    void* ptr = (void*)((char*)p->arena + p->arena_size);
    // Update the size. Make sure to keep alignment of `size_t`.
    p->arena_size += (size + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);
    return ptr;
}

#define ALLOC(p, type) (type*)alloc(p, sizeof(type))

Token get_token(Parser* p) {
    return p->get_token(p->get_token_ctx);
}

AstError* err(Parser* p, TextSpan span, const char* message) {
    size_t message_len = strlen(message);
    AstError* ast = alloc(p, sizeof(AstError) + message_len + 1);
    ast->art.kind = AST_ERROR;
    ast->art.span = span;
    memcpy(ast->message, message, message_len + 1);
    return ast;
}

Ast* parse_int(Parser* p, Token t) {
#define MSG_INVALID "Invalid integer literal"
#define MSG_RANGE "Integer literal is too big or too small"
#define MSG_UNKNOWN "Unknown error while parsing integer literal"
    char* end;
    intmax_t i = strtoimax(t.text, &end, 0);
    if (errno) {
        switch (errno) {
            case EINVAL: return (Ast*)err(p, t.span, MSG_INVALID);
            case ERANGE: return (Ast*)err(p, t.span, MSG_RANGE);
            default: return (Ast*)err(p, t.span, MSG_UNKNOWN);
        }
    } else if (end != t.text + token_len(&t)) {
        return (Ast*)err(p, t.span, MSG_INVALID);
    } else if (i < INT64_MIN || i > INT64_MAX) {
        return (Ast*)err(p, t.span, MSG_RANGE);
    }
    AstInt* ast = ALLOC(p, AstInt);
    ast->ast.kind = AST_INT;
    ast->ast.span = t.span;
    ast->value = (int64_t)i;
    return (Ast*)ast;
}

Ast* parse_var(Parser* p, Token t) {
    AstVar* ast = ALLOC(p, AstVar);
    ast->ast.kind = AST_VAR;
    ast->ast.span = t.span;
    size_t name_len = token_len(&t);
    memcpy(ast->name, t.text, name_len);
    ast->name[name_len] = '\0';
    return (Ast*)ast;
}

Ast* parser_parse(Parser* p) {
    const Token t = get_token(p);
    switch (t.kind) {
        case TOKEN_KIND_INT: return parse_int(p, t);
        case TOKEN_KIND_WORD: return parse_var(p, t);
        default: return (Ast*)err(p, t.span, "Unexpected token");
    }
}
