#include "interpreter.h"
#include "basic.h"
#include "str.h"

Interpreter interpreter_new(void) {
    return (Interpreter) { { array_empty() } };
}

static void var_table_entry_free(VarTableEntry* self) {
    free(self->name);
    *self = (VarTableEntry) { 0 };
}

static void var_table_free(VarTable* self) {
    for (size_t i = 0; i < self->entries.len; i++) {
        var_table_entry_free(&self->entries.ptr[i]);
    }
    array_free(&self->entries);
}

void interpreter_free(Interpreter* self) {
    var_table_free(&self->var_table);
}

void interpreter_add_var(Interpreter* self, char* name, Value value) {
    array_push(&self->var_table.entries, ((VarTableEntry) { name, value }));
}

bool interpreter_get_var(
    const Interpreter* self, const char* name, Value* out
) {
    ARRAY_FOREACH(&self->var_table.entries, entry) {
        if (str_eq(entry->name, name)) {
            *out = entry->value;
            return true;
        }
    }
    return false;
}

#define eval(ast) interpreter_eval_expr(i, ast)

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

static Value var_eval(Interpreter* i, const AstVar* ast) {
    Value value;
    if (!interpreter_get_var(i, ast->name, &value)) {
        PANIC("cannot evaluate variable '%s'", ast->name);
    }
    return value;
}

static Value bin_op_eval(Interpreter* i, const AstBinOp* ast) {
    const Value left = eval(ast->left);
    const Value right = eval(ast->right);
    Value result;
    if (!try_apply_bin_op(ast->op, left, right, &result)) {
        char* ast_str = ast_to_str((Ast*)ast);
        PANIC("cannot evaluate AST:\n%s", ast_str);
    }
    return result;
}

static Value unary_op_eval(Interpreter* i, const AstUnaryOp* ast) {
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

Value interpreter_eval_expr(Interpreter* i, const Ast* ast) {
    switch (ast->kind) {
        case AST_INT: return value_int(((AstInt*)ast)->value);
        case AST_VAR: return var_eval(i, (AstVar*)ast);
        case AST_BIN_OP: return bin_op_eval(i, (AstBinOp*)ast);
        case AST_UNARY_OP: return unary_op_eval(i, (AstUnaryOp*)ast);
        case AST_ASSIGN:
        case AST_NULL:
        case AST_ERROR:
            char* ast_str = ast_to_str(ast);
            PANIC("cannot evaluate AST:\n%s", ast_str);
    }
}

void interpreter_execute_statement(Interpreter* i, const Ast* ast) {
    switch (ast->kind) {
        case AST_ASSIGN: {
            const AstAssign* assign = (AstAssign*)ast;
            const Value value = eval(assign->rhs);
            interpreter_add_var(i, str_clone(assign->var), value);
            break;
        }
        case AST_INT:
        case AST_VAR:
        case AST_BIN_OP:
        case AST_UNARY_OP:
        case AST_NULL:
        case AST_ERROR: {
            char* ast_str = ast_to_str(ast);
            PANIC("cannot execute AST:\n%s", ast_str);
        }
    }
}
