#pragma once
#include "array.h"
#include "ast.h"
#include "value.h"

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
} Interpreter;

Interpreter interpreter_new(void);

void interpreter_free(Interpreter*);

void interpreter_add_var(Interpreter*, char* name, Value value);

bool interpreter_get_var(const Interpreter*, const char* name, Value* out);

Value interpreter_eval_expr(Interpreter*, const Ast*);

void interpreter_execute_statement(Interpreter*, const Ast*);
