#pragma once
#include "text_pos.h"
#include <stdbool.h>

// =============================================================================
//                                Main Ast Types
// =============================================================================

typedef enum AstKind {
    AST_NULL,
    AST_ERROR,
    // Expressions
    AST_INT,
    AST_VAR,
    AST_BIN_OP,
    AST_UNARY_OP,
    AST_FUNC,
    // Statements
    AST_ASSIGN,
    AST_COMPOUND_STATEMENT,
    AST_EMPTY_STATEMENT,
    // Programs
    AST_PROGRAM,
} AstKind;

typedef struct Ast {
    AstKind kind;
    TextSpan span;
} Ast;

// ------ Functions ------------------------------------------------------------

const char* ast_kind_name(AstKind);

char* ast_to_str(const Ast*);

// =============================================================================
//                                   Operators
// =============================================================================

// ------ Unary Operators ------------------------------------------------------

typedef enum UnaryOp {
    UNARY_OP_NEG,
    UNARY_OP_POS,
} UnaryOp;

const char* unary_op_name(UnaryOp);

const char* unary_op_symbol(UnaryOp);

// ------ Binary Operators -----------------------------------------------------

typedef enum BinOp {
    BIN_OP_ADD,
    BIN_OP_SUB,
    BIN_OP_MUL,
    BIN_OP_DIV,
    BIN_OP_REM,
} BinOp;

const char* bin_op_name(BinOp);

const char* bin_op_symbol(BinOp);

/// Higher is tighter binding.
typedef uint16_t BinOpPrecedence;

BinOpPrecedence bin_op_precedence(BinOp);

bool bin_op_is_right_associative(BinOp);

// =============================================================================
//                                Ast "Sub-Types"
// =============================================================================

typedef struct AstError {
    /// TODO: rename
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

typedef struct AstUnaryOp {
    Ast ast;
    UnaryOp op;
    Ast* arg;
} AstUnaryOp;

typedef struct AstAssign {
    Ast ast;
    Ast* rhs;
    char var[];
} AstAssign;

typedef struct AstCompoundStatement {
    Ast ast;
    size_t n_children;
    Ast* children[];
} AstCompoundStatement;

typedef Ast AstEmptyStatement;

typedef struct AstProgram {
    Ast ast;
    size_t n_statements;
    Ast* statements[];
} AstProgram;

typedef struct AstFunc {
    Ast ast;
    Ast* body;
    char param_name[];
} AstFunc;
