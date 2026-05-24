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
    parser_free(&p);
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
    parser_free(&p);
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
    parser_free(&p);
}

static void test_order_of_operations(void) {
    const char* text = "a + b * c + d * e";
    Parser p = new_parser(text);
    Ast* ast = parser_parse(&p);
    char* ast_str = ast_to_str(ast);
    const char* expected = "((a + (b * c)) + (d * e))";
    EXPECT(
        str_eq(ast_str, expected),
        "expected AST '%s' on input '%s', got '%s'",
        expected,
        text,
        ast_str
    );
    parser_free(&p);
}

void test_error_on_unexpected_token(void) {
    const char* text = "   (   ";
    Parser p = new_parser(text);
    Ast* ast = parser_parse(&p);
    EXPECT(
        ast->kind == AST_ERROR,
        "expected an error on input '%s', got %s",
        text,
        ast_kind_name(ast->kind)
    );
    parser_free(&p);
}

void test_assignment(void) {
    const char* text = "x := 42;";
    Parser p = new_parser(text);
    Ast* ast = parser_parse_statement(&p);
    char* ast_str = ast_to_str(ast);
    EXPECT(
        str_eq(ast_str, text),
        "expected statement '%s', but got '%s' instead",
        text,
        ast_str
    );
    free(ast_str);
    parser_free(&p);
}

void test_compound_statement_with_empty_statement(void) {
    const char* text = "{;}";
    const char* expected = "{\n    ;\n}";
    Parser p = new_parser(text);
    Ast* ast = parser_parse_statement(&p);
    char* ast_str = ast_to_str(ast);
    EXPECT(
        str_eq(ast_str, expected),
        "expected statement '%s', but got '%s' instead",
        expected,
        ast_str
    );
    free(ast_str);
    parser_free(&p);
}

int main(void) {
    test_empty_input();
    test_int_input();
    test_var();
    test_order_of_operations();
    test_error_on_unexpected_token();
    test_assignment();
    test_compound_statement_with_empty_statement();
}
