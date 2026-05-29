#include "repl.h"
#include "array.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "str.h"
#include "type_checker.h"
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024
static bool my_get_line(CharArray* out) {
    char buf[BUF_SIZE];
    bool success = fgets(buf, BUF_SIZE, stdin) != NULL;
    if (success) array_extend(out, buf, strlen(buf));
    return success;
}

void repl(void) {
    CharArray line = array_empty();
    while (true) {
        printf("> ");
        bool success = my_get_line(&line);
        if (!success) PANIC("bad my_get_line call");
        line.ptr[strcspn(line.ptr, "\n")] = 0; // remove the \n
        // TODO: remove all starting and trailing whitespace
        if (str_eq(line.ptr, "quit") || str_eq(line.ptr, "exit")) exit(0);
        printf("txt: %s\n", line.ptr);
        Lexer l = lexer_new(line.ptr);
        Parser p = parser_new((Token(*)(void*))lexer_pop, &l);
        const Ast* ast = parser_parse_repl_line(&p);
        printf("ast: %s\n", ast_to_str(ast));

        if (parser_had_err(&p)) {
            CharArray out = array_empty();
            ast_to_err_report(ast, line.ptr, "<repl input>", &out);
            printf("%s\n", out.ptr);
            array_free(&out);
        }

        TypeChecker c = type_checker_new();
        const Type t = type_checker_infer_expr(&c, ast);
        printf("typ: %s\n", type_to_str(&t));
        Interpreter i = interpreter_new();
        const Value v = interpreter_eval_expr(&i, ast);
        printf("val: %s\n", value_to_str(v));
        interpreter_free(&i);
        type_checker_free(&c);
        parser_free(&p);
        lexer_free(&l);
        array_clear(&line);
    }
}

#ifdef BIN

int main(void) {
    repl();
    return 0;
}

#endif
