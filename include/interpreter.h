#pragma once
#include "array.h"
#include "ast.h"
#include "value.h"

// =============================================================================
//                                 Interpreter.h
// =============================================================================
// An tree-walking interpreter. Currently only supports integer arithmetic and
// variable assignment. The interpreter maintains a variable table, which just
// maps names to values. The interpreter can be in an error state, that is, have
// an error set in it's error field. In this case, certain operations will
// silently fail and return null values.
// -----------------------------------------------------------------------------

typedef struct VarTableEntry {
    char* name;
    Value value;
} VarTableEntry;

DECLARE_ARRAY(VarTableEntry, VarTableEntryArray);

typedef struct VarTable {
    VarTableEntryArray entries;
} VarTable;

typedef struct Interpreter {
    VarTable var_table;
    char* error;
    TextSpan error_span;
} Interpreter;

/// Make a new empty interpreter.
Interpreter interpreter_new(void);

/// Free all resources owned by the interpreter.
void interpreter_free(Interpreter*);

/// Adds a variable to the interpreter's variable table.
void interpreter_add_var(Interpreter*, char* name, Value value);

/// Try to get the value of a variable. Only sets the output parameter if the
/// variable is found.
bool interpreter_get_var(const Interpreter*, const char* name, Value* out);

/// Is the interpreter in an error state?
bool interpreter_has_error(const Interpreter*);

/// Set the error of the interpreter. Overwrites any existing error.
void interpreter_set_error(Interpreter*, TextSpan span, const char* fmt, ...);

void interpreter_clear_error(Interpreter*);

/// Evaluates an expression Ast and returns the result. If something went wrong,
/// the interpreter enters an error state. In this case, or if it already was in
/// an error state, this function returns null.
Value interpreter_eval_expr(Interpreter*, const Ast*);

/// Executes a statement. If something went wrong, enters an error state.
void interpreter_execute_statement(Interpreter*, const Ast*);

void interpreter_execute_program(Interpreter*, const Ast*);
