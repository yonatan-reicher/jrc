#include "type_checker.h"
#include "lexer.h"
#include "parser.h"

Ast* parse(const char* text) {
    static bool initialized = false;
    static Lexer l;
    static Parser p;

    if (initialized) {
        parser_free(&p);
        lexer_free(&l);
    }
    l = lexer_new(text);
    p = parser_new((Token(*)(void*))lexer_pop, &l);
    initialized = true;
    return parser_parse(&p);
}

static void test_infer_int_literal(void) {
    Ast* ast = parse("123");
    TypeChecker c = type_checker_new();
    Type t = type_checker_infer_expr(&c, ast);
    EXPECT(t.kind == TYPE_INT, "int literal had type '%s'", type_to_str(&t));
    type_checker_free(&c);
}

static void test_infer_simple_arith(void) {
    Ast* ast = parse("123 + 78 * 9");
    TypeChecker c = type_checker_new();
    Type t = type_checker_infer_expr(&c, ast);
    EXPECT(
        t.kind == TYPE_INT,
        "'%s' had type '%s'",
        ast_to_str(ast),
        type_to_str(&t)
    );
    type_checker_free(&c);
}

int main() {
    test_infer_int_literal();
    test_infer_simple_arith();
    return 0;
}
