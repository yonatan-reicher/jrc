#include "parser.h"
#include "basic.h"
#include "lexer.h"

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

int main(void) {
    test_empty_input();
    test_int_input();
}
