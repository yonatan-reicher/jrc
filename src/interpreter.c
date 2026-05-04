#include "interpreter.h"
#include "basic.h"

bool try_apply_bin_op(BinOp op, Value lhs, Value rhs, Value* out) {
    if (lhs.kind != VALUE_INT || rhs.kind != VALUE_INT) {
        return false;
    }
    if (op == BIN_OP_DIV && rhs.data.i == 0) {
        return false;
    }
    switch (op) {
        case BIN_OP_ADD: *out = value_int(lhs.data.i + rhs.data.i); return true;
        case BIN_OP_SUB: *out = value_int(lhs.data.i - rhs.data.i); return true;
        case BIN_OP_MUL: *out = value_int(lhs.data.i * rhs.data.i); return true;
        case BIN_OP_DIV: *out = value_int(lhs.data.i / rhs.data.i); return true;
        case BIN_OP_REM: *out = value_int(lhs.data.i % rhs.data.i); return true;
    }
}

static Value var_eval(const AstVar* ast) {
    PANIC("cannot evaluate variable '%s'", ast->name);
}

static Value bin_op_eval(const AstBinOp* ast) {
    const Value left = eval(ast->left);
    const Value right = eval(ast->right);
    Value result;
    if (!try_apply_bin_op(ast->op, left, right, &result)) {
        char* ast_str = ast_to_str((Ast*)ast);
        PANIC("cannot evaluate AST:\n%s", ast_str);
    }
    return result;
}

static Value unary_op_eval(const AstUnaryOp* ast) {
    const Value arg = eval(ast->arg);
    if (arg.kind != VALUE_INT) {
        char* ast_str = ast_to_str((Ast*)ast);
        PANIC("cannot evaluate AST:\n%s", ast_str);
    }
    switch (ast->op) {
        case UNARY_OP_NEG: return value_int(-arg.data.i);
        case UNARY_OP_POS: return arg;
    }
}

Value eval(const Ast* ast) {
    switch (ast->kind) {
        case AST_INT: return value_int(((AstInt*)ast)->value);
        case AST_VAR: return var_eval((AstVar*)ast);
        case AST_BIN_OP: return bin_op_eval((AstBinOp*)ast);
        case AST_UNARY_OP: return unary_op_eval((AstUnaryOp*)ast);
        case AST_ASSIGN:
        case AST_NULL:
        case AST_ERROR:
            char* ast_str = ast_to_str(ast);
            PANIC("cannot evaluate AST:\n%s", ast_str);
    }
}
