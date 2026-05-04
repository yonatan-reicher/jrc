#include "interpreter.h"
#include "basic.h"
#include "lexer.h"
#include "parser.h"

void test_operators(void) {
    const char* text = "1 + 2 * ---3 - 4 / 5 % 6";
    Lexer l = lexer_new(text);
    Parser p = parser_new((Token(*)(void*))lexer_pop, &l);
    Ast* ast = parser_parse(&p);
    Interpreter i = interpreter_new();
    Value v = interpreter_eval_expr(&i, ast);
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
    interpreter_free(&i);
    parser_free(&p);
    lexer_free(&l);
}

void test_var_eval_not_defined(void) {
    const char* text = "foo";
    Lexer l = lexer_new(text);
    Parser p = parser_new((Token(*)(void*))lexer_pop, &l);
    Ast* ast = parser_parse(&p);
    Interpreter i = interpreter_new();
    Value v = interpreter_eval_expr(&i, ast);
    (void)v;
}

void test_assignment(void) {
    const char* test = "foo := 42;";
    Lexer l = lexer_new(test);
    Parser p = parser_new((Token(*)(void*))lexer_pop, &l);
    Ast* ast = parser_parse_statement(&p);
    Interpreter i = interpreter_new();
    interpreter_execute_statement(&i, ast);
    Value v;
    const bool success = interpreter_get_var(&i, "foo", &v);
    EXPECT(success, "expected variable 'foo' to be defined after assignment");
    EXPECT(v.kind == VALUE_INT, "expected variable 'foo' to be an integer");
    EXPECT(v.data.i == 42, "expected variable 'foo' to have value 42");
    interpreter_free(&i);
    parser_free(&p);
    lexer_free(&l);
}

int main(void) {
    test_operators();
    test_var_eval_not_defined();
    test_assignment();
    return 0;
}
