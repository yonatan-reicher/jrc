#include "eval.h"
#include "parser.h"
#include "lexer.h"
#include "basic.h"

void test_operators(void) {
    const char* text = "1 + 2 * ---3 - 4 / 5 % 6";
    Lexer l = lexer_new(text);
    Parser p = parser_new((Token(*)(void*))lexer_pop, &l);
    Ast* ast = parser_parse(&p);
    Value v = eval(ast);
    EXPECT(
        v.kind == VALUE_INT,
        "expected an integer value on input '%s', got %d",
        text,
        v.kind
    );
    EXPECT(
        v.data.i == 1 + 2 * -(-(-3)) - 4 / 5 % 6,
        "expected value %d on input '%s', got %" PRId64,
        1 + 2 * 3 - 4 / 5 % 6,
        text,
        v.data.i
    );
    parser_free(&p);
    lexer_free(&l);
}

void test_var_eval_not_defined(void) {
    const char* text = "foo";
    Lexer l = lexer_new(text);
    Parser p = parser_new((Token(*)(void*))lexer_pop, &l);
    Ast* ast = parser_parse(&p);
    Value v = eval(ast);
    (void)v;
}

int main(void) {
    test_operators();
    test_var_eval_not_defined();
    return 0;
}
