#include "eval.h"
#include "basic.h"

Value value_int(int64_t i) {
    Value value = { VALUE_INT, { .i = i } };
    return value;
}

Value bin_op_eval(const AstBinOp* ast) {
    const Value left = eval(ast->left);
    const Value right = eval(ast->right);
    if (left.kind != VALUE_INT || right.kind != VALUE_INT) {
        char* ast_str = ast_to_str((Ast*)ast);
        PANIC(
            "cannot apply binary operator to non-integer operands:\n%s", ast_str
        );
    }
    if (ast->op == BIN_OP_DIV && right.data.i == 0) {
        char* ast_str = ast_to_str((Ast*)ast);
        PANIC("division by zero in expression:\n%s", ast_str);
    }
    if (ast->op != BIN_OP_ADD && ast->op != BIN_OP_SUB &&
        ast->op != BIN_OP_MUL && ast->op != BIN_OP_DIV &&
        ast->op != BIN_OP_REM) {
        char* ast_str = ast_to_str((Ast*)ast);
        PANIC("unknown binary operator in expression:\n%s", ast_str);
    }
    return value_int(
        (ast->op == BIN_OP_ADD)   ? left.data.i + right.data.i
        : (ast->op == BIN_OP_SUB) ? left.data.i - right.data.i
        : (ast->op == BIN_OP_MUL) ? left.data.i * right.data.i
        : (ast->op == BIN_OP_DIV) ? left.data.i / right.data.i
        : (ast->op == BIN_OP_REM) ? left.data.i % right.data.i
                                  : 0
    );
}

Value var_eval(const AstVar* ast) {
    PANIC("cannot evaluate variable '%s'", ast->name);
}

Value eval(const Ast* ast) {
    switch (ast->kind) {
        case AST_INT: return value_int(((AstInt*)ast)->value);
        case AST_VAR: return var_eval((AstVar*)ast);
        case AST_BIN_OP: return bin_op_eval((AstBinOp*)ast);
        case AST_NULL:
        case AST_ERROR:
            char* ast_str = ast_to_str(ast);
            PANIC("cannot evaluate AST:\n%s", ast_str);
            free(ast_str);
            break;
    }
}
