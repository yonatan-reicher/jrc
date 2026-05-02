#pragma once
#include "text_pos.h"
#include "lexer.h"
#include <inttypes.h>

typedef enum AstKind {
    AST_NULL,
    AST_ERROR,
    AST_INT,
    AST_VAR,
} AstKind;

typedef struct Ast {
    AstKind kind;
    TextSpan span;
} Ast;

typedef struct AstError {
    Ast art;
    char message[];
} AstError;

typedef struct AstInt {
    Ast ast;
    int64_t value;
} AstInt;

typedef struct AstVar {
    Ast ast;
    char name[];
} AstVar;

typedef struct Parser {
    // Getting tokens
    Token (*get_token)(void* ctx);
    void* get_token_ctx;
    // Managing memory
    void* arena;
    size_t arena_size;
    size_t arena_capacity;
    void** previous_arenas;
    size_t n_previous_arenas;
} Parser;

const char* ast_kind_name(AstKind kind);

Parser parser_new(Token (*get_token)(void* ctx), void* get_token_ctx);

void parser_free(Parser* parser);

Ast* parser_parse(Parser* parser);
