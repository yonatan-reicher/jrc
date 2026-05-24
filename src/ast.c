#include "ast.h"
#include "array.h"
#include "basic.h"
#include "str.h"

// =============================================================================
//                                      Ast
// =============================================================================

const char* ast_kind_name(AstKind kind) {
    switch (kind) {
        case AST_NULL: return "NULL";
        case AST_ERROR: return "ERROR";
        case AST_INT: return "INT";
        case AST_VAR: return "VAR";
        case AST_BIN_OP: return "BIN_OP";
        case AST_UNARY_OP: return "UNARY_OP";
        case AST_ASSIGN: return "ASSIGN";
        case AST_COMPOUND_STATEMENT: return "COMPOUND_STATEMENT";
        case AST_EMPTY_STATEMENT: return "EMPTY_STATEMENT";
        case AST_PROGRAM: return "PROGRAM";
    }
}

static char* ast_error_to_str(const AstError* ast) {
    return str_format(
        "<ERROR on " TEXT_SPAN_PRINTF_FORMAT ": %s>",
        TEXT_SPAN_PRINTF(ast->art.span),
        ast->message
    );
}

static char* ast_int_to_str(const AstInt* ast) {
    return str_format("%" PRId64, ast->value);
}

static char* ast_var_to_str(const AstVar* ast) {
    return str_clone(ast->name);
}

static char* ast_bin_op_to_str(const AstBinOp* ast) {
    char* lhs = ast_to_str(ast->left);
    char* rhs = ast_to_str(ast->right);
    const char* op_symbol = bin_op_symbol(ast->op);
    char* ret = str_format("(%s %s %s)", lhs, op_symbol, rhs);
    free(lhs);
    free(rhs);
    return ret;
}

static char* ast_unary_op_to_str(const AstUnaryOp* ast) {
    char* arg = ast_to_str(ast->arg);
    const char* op_symbol = unary_op_symbol(ast->op);
    char* ret = str_format("%s%s", op_symbol, arg);
    free(arg);
    return ret;
}

static char* ast_assign_to_str(const AstAssign* ast) {
    char* rhs = ast_to_str(ast->rhs);
    char* ret = str_format("%s := %s;", ast->var, rhs);
    free(rhs);
    return ret;
}

static char* ast_compound_statement_to_str(const AstCompoundStatement* ast) {
    CharArray buf = array_empty();
    array_extend(&buf, "{\n", 2);
    for (size_t i_child = 0; i_child < ast->n_children; i_child++) {
        char* child_str = ast_to_str(ast->children[i_child]);
        array_extend(&buf, "    ", 4);
        array_extend(&buf, child_str, strlen(child_str));
        array_push(&buf, '\n');
    }
    array_extend(&buf, "}", 2); // Also add the '\0'!
    return buf.ptr;
}

static char* ast_program_to_str(const AstProgram* ast) {
    if (ast->n_statements == 0) return str_clone("");
    if (ast->n_statements == 1) return ast_to_str(ast->statements[0]);
    CharArray buf = array_empty();
    for (size_t i_stmt = 0; i_stmt < ast->n_statements; i_stmt++) {
        char* stmt_str = ast_to_str(ast->statements[i_stmt]);
        array_extend(&buf, stmt_str, strlen(stmt_str));
        array_push(&buf, '\n');
        free(stmt_str);
    }
    return buf.ptr;
}

char* ast_to_str(const Ast* ast) {
    switch (ast->kind) {
        case AST_NULL: return str_clone("<NULL>");
        case AST_ERROR: return ast_error_to_str((const AstError*)ast);
        case AST_INT: return ast_int_to_str((const AstInt*)ast);
        case AST_VAR: return ast_var_to_str((const AstVar*)ast);
        case AST_BIN_OP: return ast_bin_op_to_str((const AstBinOp*)ast);
        case AST_UNARY_OP: return ast_unary_op_to_str((const AstUnaryOp*)ast);
        case AST_ASSIGN: return ast_assign_to_str((const AstAssign*)ast);
        case AST_COMPOUND_STATEMENT:
            return ast_compound_statement_to_str(
                (const AstCompoundStatement*)ast
            );
        case AST_EMPTY_STATEMENT: return str_clone(";");
        case AST_PROGRAM: return ast_program_to_str((const AstProgram*)ast);
    }
}

// =============================================================================
//                                Unary Operators
// =============================================================================

typedef struct UnaryOpInfo {
    const char* name;
    const char* symbol;
} UnaryOpInfo;

static UnaryOpInfo unary_op_info(UnaryOp op) {
    switch (op) {
        case UNARY_OP_NEG: return (UnaryOpInfo) { "NEG", "-" };
        case UNARY_OP_POS: return (UnaryOpInfo) { "POS", "+" };
    }
}

const char* unary_op_name(UnaryOp op) {
    return unary_op_info(op).name;
}

const char* unary_op_symbol(UnaryOp op) {
    return unary_op_info(op).symbol;
}

// =============================================================================
//                               Binary Operators
// =============================================================================

typedef struct BinOpInfo {
    const char* name;
    const char* symbol;
    BinOpPrecedence precedence;
    bool is_right_associative;
} BinOpInfo;

static BinOpInfo bin_op_info(BinOp op) {
    switch (op) {
        case BIN_OP_ADD: return (BinOpInfo) { "ADD", "+", 50, false };
        case BIN_OP_SUB: return (BinOpInfo) { "SUB", "-", 50, false };
        case BIN_OP_MUL: return (BinOpInfo) { "MUL", "*", 100, false };
        case BIN_OP_DIV: return (BinOpInfo) { "DIV", "/", 100, false };
        case BIN_OP_REM: return (BinOpInfo) { "REM", "%", 100, false };
    }
}

const char* bin_op_name(BinOp op) {
    return bin_op_info(op).name;
}

const char* bin_op_symbol(BinOp op) {
    return bin_op_info(op).symbol;
}

BinOpPrecedence bin_op_precedence(BinOp op) {
    return bin_op_info(op).precedence;
}

bool bin_op_is_right_associative(BinOp op) {
    return bin_op_info(op).is_right_associative;
}
