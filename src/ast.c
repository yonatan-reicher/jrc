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
        case AST_FUNC: return "FUNC";
    }
}

void ast_children(const Ast* ast, ConstAstPtrArray* out_children) {
    switch (ast->kind) {
        // No children
        case AST_NULL:
        case AST_ERROR:
        case AST_INT:
        case AST_VAR:
        case AST_EMPTY_STATEMENT: break;
        // Single children
        case AST_UNARY_OP:
            array_push(out_children, ((const AstUnaryOp*)ast)->arg);
            break;
        case AST_ASSIGN:
            array_push(out_children, ((const AstAssign*)ast)->rhs);
            break;
        case AST_FUNC:
            array_push(out_children, ((const AstFunc*)ast)->body);
            break;
        // Twins
        case AST_BIN_OP:
            array_push(out_children, ((const AstBinOp*)ast)->left);
            array_push(out_children, ((const AstBinOp*)ast)->right);
            break;
        // More
        case AST_COMPOUND_STATEMENT:
            const AstCompoundStatement* ast_ = (const AstCompoundStatement*)ast;
            array_extend(out_children, ast_->children, ast_->n_children);
            break;
        case AST_PROGRAM:
            const AstProgram* ast__ = (const AstProgram*)ast;
            array_extend(out_children, ast__->statements, ast__->n_statements);
            break;
    }
}

void ast_each(const Ast* ast, void f(const Ast*, void*), void* f_data) {
    f(ast, f_data);
    ConstAstPtrArray children = array_empty();
    ast_children(ast, &children);
    ARRAY_FOREACH(&children, child) ast_each(*child, f, f_data);
    array_free(&children);
}

static char* ast_error_to_str(const AstError* ast) {
    return str_format(
        "<ERROR on " TEXT_SPAN_PRINTF_FORMAT ": %s>",
        TEXT_SPAN_PRINTF(ast->ast.span),
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

static char* ast_func_to_str(const AstFunc* ast) {
    char* body_str = ast_to_str(ast->body);
    char* ret = str_format("(%s => %s)", ast->param_name, body_str);
    free(body_str);
    return ret;
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
        case AST_FUNC: return ast_func_to_str((const AstFunc*)ast);
    }
}

/// Returns the full lines in the text that contain the given span, with
/// additional lines above or below. The span is marked via '>' and '<'.
static char* get_part_that_matters(
    const char* text, TextSpan span, int context
) {
    const char *span_start = &text[span.start.index],
               *span_end = &text[span.end.index];
    const char *start = span_start, *end = span_end;
    // Go back in start,
    for (int i = 0; i < context + 1; i++)
        while (start > text && *(start - 1) != '\n') start--;
    // and forwards in end.
    for (int i = 0; i < context + 1; i++)
        while (*end != 0 && *end != '\n') end++;
    char* ret = str_format(
        "%.*s>%.*s<%.*s",
        (int)(span_start - start),
        start,
        (int)(span_end - span_start),
        span_start,
        (int)(end - span_end),
        span_end
    );
    return ret;
}

static void collect_err(const Ast*, ConstAstPtrArray*);

void ast_to_err_report(
    const Ast* ast, const char* text, const char* file, CharArray* out
) {
    ConstAstPtrArray err_asts = array_empty();
    ast_each(ast, (void (*)(const Ast*, void*))collect_err, &err_asts);
#define EMIT_STR(S)                                                            \
    do {                                                                       \
        const char* _s = (S);                                                  \
        array_extend(out, _s, strlen(_s));                                     \
    } while (0)
#define EMIT_AND_FREE_STR(S)                                                   \
    do {                                                                       \
        char* __s = (S);                                                       \
        EMIT_STR(__s);                                                         \
        free(__s);                                                             \
    } while (0)
    // Actually writing the error
    ARRAY_FOREACH(&err_asts, x) {
        const AstError* err_ast = (const AstError*)*x;
        EMIT_AND_FREE_STR(str_format(
            "%s" TEXT_SPAN_PRINTF_FORMAT ": ",
            file,
            TEXT_SPAN_PRINTF(err_ast->ast.span)
        ));
        EMIT_STR("error: "); // TODO: Make this red?
        EMIT_STR(err_ast->message);
        EMIT_STR("\n\n");
        EMIT_AND_FREE_STR(get_part_that_matters(text, err_ast->ast.span, 1));
    }
    array_push(out, 0);
    // Done
#undef EMIT_AND_FREE_STR
#undef EMIT_STR
    array_free(&err_asts);
}

static void collect_err(const Ast* ast, ConstAstPtrArray* out) {
    if (ast->kind == AST_ERROR) array_push(out, ast);
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
