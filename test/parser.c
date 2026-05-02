#include "parser.h"
#include "basic.h"
#include "lexer.h"
#include "str.h"

Parser new_parser(const char* text) {
    Lexer* l = malloc(sizeof(Lexer));
    *l = lexer_new(text);
    return parser_new((Token(*)(void*))lexer_pop, l);
}

void test_empty_input(void) {
    const char* text = "";
    Parser p = new_parser(text);
    Ast* ast = parser_parse(&p);
    EXPECT(
        ast->kind == AST_ERROR,
        "expected an error on empty input, got %s",
        ast_kind_name(ast->kind)
    );
}

void test_int_input(void) {
    const char* text = "   123   ";
    Parser p = new_parser(text);
    Ast* ast = parser_parse(&p);
    EXPECT(
        ast->kind == AST_INT,
        "expected an integer on input '%s', got %s",
        text,
        ast_kind_name(ast->kind)
    );
    EXPECT(
        ((AstInt*)ast)->value == 123,
        "expected value 123 on input '%s', got %" PRId64,
        text,
        ((AstInt*)ast)->value
    );
}

static void test_var(void) {
    const char* text = "   foo   ";
    Parser p = new_parser(text);
    Ast* ast = parser_parse(&p);
    EXPECT(
        ast->kind == AST_VAR,
        "expected a variable on input '%s', got %s",
        text,
        ast_kind_name(ast->kind)
    );
    EXPECT(
        str_eq(((AstVar*)ast)->name, "foo"),
        "expected variable name 'foo' on input '%s', got '%s'",
        text,
        ((AstVar*)ast)->name
    );
}

static void test_order_of_operations(void) {
    const char* text = "a + b * c + d * e";
    Parser p = new_parser(text);
    Ast* ast = parser_parse(&p);
    // clang-format off
    EXPECT(ast->kind == AST_BIN_OP, "root should be an operator");
    AstBinOp* ast_ = (AstBinOp*)ast;
    Ast *left = ast_->left, *right = ast_->right;
    EXPECT(ast_->op == BIN_OP_ADD, "root should be addition");
    EXPECT(left->kind == AST_VAR, "left child should be a variable");
    AstVar* left_ = (AstVar*)left;
    EXPECT(str_eq(left_->name, "a"), "left variable should be 'a'");
    EXPECT(right->kind == AST_BIN_OP, "right child should be an operator");
    AstBinOp* right_ = (AstBinOp*)right;
    Ast *right_left = right_->left, *right_right = right_->right;
    EXPECT(right_->op == BIN_OP_ADD, "right child should be addition");
    EXPECT(right_left->kind == AST_BIN_OP, "right left child should be an operator");
    AstBinOp* right_left_ = (AstBinOp*)right_left;
    EXPECT(right_left_->op == BIN_OP_MUL, "right left child should be multiplication");
    EXPECT(right_left_->left->kind == AST_VAR, "right left left child should be a variable");
    AstVar* right_left_left_ = (AstVar*)right_left_->left;
    EXPECT(str_eq(right_left_left_->name, "b"), "right left left variable should be 'b'");
    EXPECT(right_left_->right->kind == AST_VAR, "right left right child should be a variable");
    AstVar* right_left_right_ = (AstVar*)right_left_->right;
    EXPECT(str_eq(right_left_right_->name, "c"), "right left right variable should be 'c'");
    EXPECT(right_right->kind == AST_BIN_OP, "right right child should be an operator");
    AstBinOp* right_right_ = (AstBinOp*)right_right;
    EXPECT(right_right_->op == BIN_OP_MUL, "right right child should be multiplication");
    EXPECT(right_right_->left->kind == AST_VAR, "right right left child should be a variable");
    AstVar* right_right_left_ = (AstVar*)right_right_->left;
    EXPECT(str_eq(right_right_left_->name, "d"), "right right left variable should be 'd'");
    EXPECT(right_right_->right->kind == AST_VAR, "right right right child should be a variable");
    AstVar* right_right_right_ = (AstVar*)right_right_->right;
    EXPECT(str_eq(right_right_right_->name, "e"), "right right right variable should be 'e'");
    // clang-format on
}

int main(void) {
    test_empty_input();
    test_int_input();
    test_var();
    test_order_of_operations();
}
