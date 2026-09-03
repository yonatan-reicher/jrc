#include "interpreter.h"
#include "basic.h"
#include "str.h"
#include <stdarg.h>

// =============================================================================
//                                    Helpers
// =============================================================================

// Let's define some shortcuts for this document
#define eval(ast) interpreter_eval_expr(i, ast)
#define execute(ast) interpreter_execute_statement(i, ast)
#define set_error(span, fmt, ...)                                              \
    interpreter_set_error(i, span, fmt, __VA_ARGS__)

#define TODO PANIC("todo")

typedef InterpreterScope Scope;

// =============================================================================
//                             Variable Table Entry
// =============================================================================

static void var_table_entry_free(VarTableEntry* self) {
    if (self == NULL) return;
    free(self->name);
    *self = (VarTableEntry) { 0 };
}

// =============================================================================
//                                Variable Table
// =============================================================================

static VarTable var_table_new(void) {
    return (VarTable) { array_empty() };
}

static void var_table_free(VarTable* self) {
    if (self == NULL) return;
    ARRAY_FOREACH(&self->entries, x) var_table_entry_free(x);
    array_free(&self->entries);
}

// =============================================================================
//                                    Scopes
// =============================================================================

static Scope scope_new(void) {
    return (Scope) { var_table_new() };
}

static void scope_free(Scope* s) {
    if (s == NULL) return;
    var_table_free(&s->var_table);
    *s = (Scope) { 0 };
}

static void scope_add_var(Scope* s, char* name, Value value) {
    array_push(&s->var_table.entries, ((VarTableEntry) { name, value }));
}

static bool scope_get_var(Scope* s, const char* name, Value* out) {
    ARRAY_FOREACH_REV(&s->var_table.entries, entry) {
        if (str_eq(entry->name, name)) {
            *out = entry->value;
            return true;
        }
    }
    return false;
}

// =============================================================================
//                                  Interpreter
// =============================================================================

// ------ Private --------------------------------------------------------------

/// Returns the scope at the top of the scope stack. Macro because of
/// const-correctness.
#define current_scope(I) array_last(&(I)->scopes)

// ------ Public ---------------------------------------------------------------

Interpreter interpreter_new(void) {
    Interpreter ret = { array_empty(), NULL, {} };
    array_push(&ret.scopes, scope_new());
    return ret;
}

void interpreter_free(Interpreter* self) {
    ARRAY_FOREACH(&self->scopes, x) scope_free(x);
    array_free(&self->scopes);
    *self = (Interpreter) { 0 };
}

void interpreter_add_var(Interpreter* self, char* name, Value value) {
    scope_add_var(current_scope(self), name, value);
}

bool interpreter_get_var(
    const Interpreter* self, const char* name, Value* out
) {
    return scope_get_var(current_scope(self), name, out);
}

bool interpreter_has_error(const Interpreter* self) {
    return self->error != NULL;
}

void interpreter_set_error(
    Interpreter* self, TextSpan span, const char* fmt, ...
) {
    va_list args;
    va_start(args, fmt);
    char* error = str_format_va_list(fmt, args);
    va_end(args);
    free(self->error);
    self->error = error;
    self->error_span = span;
}

void interpreter_clear_error(Interpreter* self) {
    free(self->error);
    self->error = NULL;
    self->error_span = (TextSpan) { 0 };
}

static bool try_apply_bin_op(BinOp op, Value lhs, Value rhs, Value* out) {
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
    PANIC("Bad BinOp: %d", op);
}

static Value var_eval(Interpreter* i, const AstVar* ast) {
    Value value;
    if (!interpreter_get_var(i, ast->name, &value)) {
        set_error(ast->ast.span, "undefined variable '%s'", ast->name);
        return value_null();
    }
    return value;
}

static Value bin_op_eval(Interpreter* i, const AstBinOp* ast) {
    const Value left = eval(ast->left);
    const Value right = eval(ast->right);
    if (interpreter_has_error(i)) return value_null();
    Value result;
    if (!try_apply_bin_op(ast->op, left, right, &result)) {
        char* left_str = value_to_str(left);
        char* right_str = value_to_str(right);
        set_error(
            ast->ast.span,
            "cannot apply binary operator '%s' to values '%s' and '%s'",
            bin_op_symbol(ast->op),
            left_str,
            right_str
        );
        free(left_str);
        free(right_str);
        return value_null();
    }
    return result;
}

static Value unary_op_eval(Interpreter* i, const AstUnaryOp* ast) {
    const Value arg = eval(ast->arg);
    if (interpreter_has_error(i)) return value_null();
    if (arg.kind != VALUE_INT) {
        char* arg_str = value_to_str(arg);
        set_error(
            ast->ast.span,
            "cannot apply unary operator '%s' to value '%s'",
            unary_op_symbol(ast->op),
            arg_str
        );
        free(arg_str);
        return value_null();
    }
    switch (ast->op) {
        case UNARY_OP_NEG: return value_int(-arg.data.i);
        case UNARY_OP_POS: return arg;
    }
    PANIC("Bad UnaryOp: %d", ast->op);
}

Value eval_func(Interpreter* i, const AstFunc* ast) {
    TODO;
}

Value interpreter_eval_expr(Interpreter* i, const Ast* ast) {
    if (interpreter_has_error(i)) return value_null();
    switch (ast->kind) {
        case AST_INT: return value_int(((const AstInt*)ast)->value);
        case AST_VAR: return var_eval(i, (const AstVar*)ast);
        case AST_BIN_OP: return bin_op_eval(i, (const AstBinOp*)ast);
        case AST_UNARY_OP: return unary_op_eval(i, (const AstUnaryOp*)ast);
        case AST_FUNC: return eval_func(i, (const AstFunc*)ast);
        case AST_ASSIGN:
        case AST_COMPOUND_STATEMENT:
        case AST_EMPTY_STATEMENT:
        case AST_PROGRAM:
        case AST_NULL:
        case AST_ERROR:
            char* ast_str = ast_to_str(ast);
            PANIC("cannot evaluate AST:\n%s", ast_str);
    }
    PANIC("Bad AstKind: %d", ast->kind);
}

void interpreter_execute_statement(Interpreter* i, const Ast* ast) {
    if (interpreter_has_error(i)) return;
    switch (ast->kind) {
        case AST_ASSIGN: {
            const AstAssign* assign = (const AstAssign*)ast;
            const Value value = eval(assign->rhs);
            interpreter_add_var(i, str_clone(assign->var), value);
            break;
        }
        case AST_COMPOUND_STATEMENT: {
            const AstCompoundStatement* compound =
                (const AstCompoundStatement*)ast;
            for (size_t i_child = 0; i_child < compound->n_children;
                 i_child++) {
                interpreter_execute_statement(i, compound->children[i_child]);
                if (interpreter_has_error(i)) break;
            }
            break;
        }
        case AST_EMPTY_STATEMENT: break;
        case AST_PROGRAM:
        case AST_INT:
        case AST_VAR:
        case AST_BIN_OP:
        case AST_UNARY_OP:
        case AST_FUNC:
        case AST_NULL:
        case AST_ERROR: {
            char* ast_str = ast_to_str(ast);
            PANIC("cannot execute AST:\n%s", ast_str);
        }
    }
}

void interpreter_execute_program(Interpreter* i, const Ast* a) {
    EXPECT(
        a->kind == AST_PROGRAM,
        "expected a program AST, got %s",
        ast_kind_name(a->kind)
    );
    const AstProgram* program = (const AstProgram*)a;
    for (size_t i_stmt = 0; i_stmt < program->n_statements; i_stmt++) {
        interpreter_execute_statement(i, program->statements[i_stmt]);
        if (interpreter_has_error(i)) return;
    }
}
