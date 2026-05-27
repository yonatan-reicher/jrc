#include "type_checker.h"
#include "array.h"
#include "ast.h"
#include "str.h"

typedef TypeChecker This;
typedef TypeCheckerVarTable VarTable;
typedef TypeCheckerVarTableEntry VarTableEntry;

static VarTableEntry* var_table_get_entry(VarTable* this, const char* name) {
    ARRAY_FOREACH_REV(&this->entries, entry) {
        if (str_eq(entry->name, name)) return entry;
    }
    return NULL;
}

TypeChecker type_checker_new(void) {
    TypeChecker this = { { array_empty() }, NULL };
    const TypeCheckerVarTableEntry e = { str_clone("int"), type_int() };
    array_push(&this.var_table.entries, e);
    return this;
}

void type_checker_free(TypeChecker* this) {
    if (this->error_message) free(this->error_message);
    ARRAY_FOREACH(&this->var_table.entries, entry) free(entry->name);
    array_free(&this->var_table.entries);
    *this = (TypeChecker) { 0 };
}

bool type_checker_has_error(const TypeChecker* this) {
    return this->error_message != NULL;
}
const char* type_checker_get_error(const TypeChecker* this) {
    return this->error_message;
}

#define set_error(fmt, ...)                                                    \
    (this->error_message = this->error_message ? this->error_message           \
                                               : str_format(fmt, __VA_ARGS__))

#define infer_expr type_checker_infer_expr
#define has_error type_checker_has_error

static Type infer_var(This* this, const AstVar* ast) {
    VarTableEntry* entry = var_table_get_entry(&this->var_table, ast->name);
    if (!entry) {
        set_error("undefined variable '%s'", ast->name);
        return type_error();
    }
    return entry->type;
}

static Type infer_bin_op(This* this, const AstBinOp* ast) {
    const Type left_type = infer_expr(this, ast->left);
    const Type right_type = infer_expr(this, ast->right);
    if (has_error(this)) return type_int();
    if (left_type.kind == TYPE_INT && right_type.kind == TYPE_INT) {
        return type_int();
    }
    char* left_type_str = type_to_str(&left_type);
    char* right_type_str = type_to_str(&right_type);
    set_error(
        "cannot apply binary operator '%s' to types '%s' and '%s'",
        bin_op_symbol(ast->op),
        left_type_str,
        right_type_str
    );
    free(left_type_str);
    free(right_type_str);
    return type_error();
}

static Type infer_unary_op(This* this, const AstUnaryOp* ast) {
    Type arg_type = infer_expr(this, ast->arg);
    if (has_error(this)) return type_error();
    if (arg_type.kind == TYPE_INT) {
        return type_int();
    }
    char* arg_str = type_to_str(&arg_type);
    set_error(
        "cannot apply unary '%s' to type '%s'",
        unary_op_symbol(ast->op),
        arg_str
    );
    return type_error();
}

Type infer_expr(This* this, const Ast* ast) {
    if (this->error_message) return type_int();
    switch (ast->kind) {
        case AST_INT: return type_int();
        case AST_VAR: return infer_var(this, (const AstVar*)ast);
        case AST_BIN_OP: return infer_bin_op(this, (const AstBinOp*)ast);
        case AST_UNARY_OP: return infer_unary_op(this, (const AstUnaryOp*)ast);
        case AST_NULL:
        case AST_ERROR:
        case AST_FUNC:
        case AST_ASSIGN:
        case AST_COMPOUND_STATEMENT:
        case AST_EMPTY_STATEMENT:
        case AST_PROGRAM:
            PANIC("unhandled ast kind %s", ast_kind_name(ast->kind)); // TODO
    }
}

void type_checker_check_expr(
    This* this, const Ast* ast, const Type* expected_type
) {
    if (has_error(this)) return;
    Type actual_type = infer_expr(this, ast);
    if (!type_eq(&actual_type, expected_type)) {
        char* ast_str = ast_to_str(ast);
        char* actual_type_str = type_to_str(&actual_type);
        char* expected_type_str = type_to_str(expected_type);
        set_error(
            "type mismatch: '%s' has type '%s' but was expected to have type "
            "'%s'",
            ast_str,
            actual_type_str,
            expected_type_str
        );
        free(ast_str);
        free(actual_type_str);
        free(expected_type_str);
    }
    return;
}
