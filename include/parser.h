#pragma once
#include "text_pos.h"
#include "token.h"
#include <inttypes.h>

// =============================================================================
//                                   Operators
// =============================================================================

typedef enum BinOp {
    BIN_OP_ADD,
    BIN_OP_SUB,
    BIN_OP_MUL,
    BIN_OP_DIV,
    BIN_OP_REM,
} BinOp;

/// Higher is tighter binding.
typedef uint16_t BinOpPrecedence;

BinOpPrecedence bin_op_precedence(BinOp op);

// =============================================================================
//                                   Ast Types
// =============================================================================

typedef enum AstKind {
    AST_NULL,
    AST_ERROR,
    AST_INT,
    AST_VAR,
    AST_BIN_OP,
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

typedef struct AstBinOp {
    Ast ast;
    BinOp op;
    Ast* left;
    Ast* right;
} AstBinOp;

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

const char* ast_kind_name(AstKind kind);

Parser parser_new(Token (*get_token)(void* ctx), void* get_token_ctx);

void parser_free(Parser* parser);

Ast* parser_parse(Parser* parser);
