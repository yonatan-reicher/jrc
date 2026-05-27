#pragma once
#include "array.h"
#include "ast.h"
#include "type.h"
#include <stdbool.h>

// =============================================================================
//                                type_checker.h
// =============================================================================
// The `TypeChecker` struct is responsible for checking the validity of ASTs and
// inferring their types. It implements a simple bi-directional type checking
// static analysis.
// When something goes wrong, an error is saved in the object and can be
// retrieved with appropriate functions.
// =============================================================================

typedef struct TypeCheckerVarTableEntry {
    char* name;
    Type type;
} TypeCheckerVarTableEntry;

DECLARE_ARRAY(TypeCheckerVarTableEntry, TypeCheckerVarTableEntryArray);

typedef struct TypeCheckerVarTable {
    TypeCheckerVarTableEntryArray entries;
} TypeCheckerVarTable;

typedef struct TypeChecker {
    TypeCheckerVarTable var_table;
    char* error_message;
} TypeChecker;

TypeChecker type_checker_new(void);

void type_checker_free(TypeChecker*);

bool type_checker_has_error(const TypeChecker* this);
const char* type_checker_get_error(const TypeChecker* this);

Type type_checker_infer_expr(TypeChecker*, const Ast*);
void type_checker_check_expr(
    TypeChecker*, const Ast*, const Type* expected_type
);

void type_checker_check_statement(TypeChecker*, const Ast*);

void type_checker_check_program(TypeChecker*, const Ast*);
