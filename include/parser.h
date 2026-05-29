#pragma once
#include "ast.h"
#include "token.h"
#include <inttypes.h>

typedef struct Parser {
    // Getting tokens
    Token (*get_token)(void* ctx);
    void* get_token_ctx;
    int n_tokens_peeked;
    Token peeked_tokens[2];
    // Managing memory
    void* arena;
    size_t arena_size;
    size_t arena_capacity;
    void** previous_arenas;
    size_t n_previous_arenas;
    // Remembering things
    bool had_err;
} Parser;

// =============================================================================
//                                   Functions
// =============================================================================

Parser parser_new(Token (*get_token)(void* ctx), void* get_token_ctx);

void parser_free(Parser* parser);

Ast* parser_parse_expr(Parser* parser);

Ast* parser_parse_statement(Parser*);

Ast* parser_parse_program(Parser*);

Ast* parser_parse_repl_line(Parser*);

// Error

bool parser_had_err(const Parser* p);

void parser_clear_err(Parser* p);
