#include "type_checker.h"
#include "lexer.h"
#include "parser.h"

Ast* parse_expr(const char* text) {
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

Ast* parse_program(const char* text) {
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
    return parser_parse_program(&p);
}

static void test_infer_int_literal(void) {
    Ast* ast = parse_expr("123");
    TypeChecker c = type_checker_new();
    Type t = type_checker_infer_expr(&c, ast);
    EXPECT(t.kind == TYPE_INT, "int literal had type '%s'", type_to_str(&t));
    type_checker_free(&c);
}

static void test_infer_simple_arith(void) {
    Ast* ast = parse_expr("123 + 78 * 9");
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

static void test_check_assign(void) {
    Ast* ast = parse_program(""
                     "x := 1 + 2;\n"
                     "x := 3;\n"
                     "");
    TypeChecker c = type_checker_new();
    type_checker_check_program(&c, ast);
    if (type_checker_has_error(&c)) {
        PANIC("%s", type_checker_get_error(&c));
    }
    type_checker_free(&c);
}

int main() {
    test_infer_int_literal();
    test_infer_simple_arith();
    test_check_assign();
    return 0;
}
