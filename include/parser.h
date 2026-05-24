#pragma once
#include "ast.h"
#include "token.h"
#include <inttypes.h>

typedef struct Parser {
    // Getting tokens
    Token (*get_token)(void* ctx);
    void* get_token_ctx;
    bool has_peeked_token;
    Token peeked_token;
    // Managing memory
    void* arena;
    size_t arena_size;
    size_t arena_capacity;
    void** previous_arenas;
    size_t n_previous_arenas;
} Parser;

// =============================================================================
//                                   Functions
// =============================================================================

Parser parser_new(Token (*get_token)(void* ctx), void* get_token_ctx);

void parser_free(Parser* parser);

/// TODO: Rename to `parser_parse_expr`
Ast* parser_parse(Parser* parser);

Ast* parser_parse_statement(Parser*);
