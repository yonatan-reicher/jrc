#include "value.h"
#include "basic.h"
#include "str.h"

void test_no_captured_to_str(void) {
    AstInt lhs = {
        { AST_INT, {} },
        42
    };
    AstVar* rhs = malloc(sizeof(AstVar) + 10);
    *rhs = (AstVar) {
        { AST_VAR, {} },
    };
    strcpy(rhs->name, "x");
    AstBinOp root = {
        { AST_BIN_OP, {} },
        BIN_OP_ADD,
        (Ast*)&lhs,
        (Ast*)rhs
    };
    Value f = value_func_new("x", (Ast*)&root, 0, NULL, NULL);
    char* s = value_to_str(f);
    EXPECT(str_eq(s, ""), "");
    free(s);
    free(rhs);
}

int main(void) {
    return 0;
}
