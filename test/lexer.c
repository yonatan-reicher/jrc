#include "lexer.h"
#include "basic.h"

bool token_eq(const Token* a, const Token* b) {
    return a->kind == b->kind && a->span.start.index == b->span.start.index &&
           a->span.start.row == b->span.start.row &&
           a->span.start.col == b->span.start.col &&
           a->span.end.index == b->span.end.index &&
           a->span.end.row == b->span.end.row &&
           a->span.end.col == b->span.end.col;
}

#define EXPECT_TOKEN_EQ(A, B)                                                  \
    do {                                                                       \
        if (!token_eq(&(A), &(B))) {                                           \
            PANIC(                                                             \
                "Expected " TOKEN_PRINTF_FORMAT                                \
                " but got " TOKEN_PRINTF_FORMAT " instead.",                   \
                TOKEN_PRINTF((B)),                                             \
                TOKEN_PRINTF((A))                                              \
            );                                                                 \
        }                                                                      \
    } while (0)

Token new_token(
    TokenKind kind,
    size_t start_index,
    TextRow start_row,
    TextCol start_col,
    int len
) {
    return (Token) {
        .kind = kind,
        .span = {
            .start = {
                .index = start_index,
                .row = start_row,
                .col = start_col,
            },
            .end = {
                .index = start_index + len,
                .row = start_row,
                .col = start_col + len,
            },
        },
    };
}

#define NEW_TOKEN(KIND, START_INDEX, START_ROW, START_COL, LEN)                \
    new_token(TOKEN_KIND_##KIND, START_INDEX, START_ROW, START_COL, LEN)

void test_eof(void) {
    Lexer lexer = lexer_new("");
    const Token token = lexer_pop(&lexer);
    const Token expected = NEW_TOKEN(EOF, 0, 1, 1, 0);
    if (!token_eq(&token, &expected)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT
            " instead.",
            TOKEN_PRINTF(expected),
            TOKEN_PRINTF(token)
        );
    }
}

void test_single_int(void) {
    Lexer lexer = lexer_new("123");
    const Token token = lexer_pop(&lexer);
    const Token expected = NEW_TOKEN(INT, 0, 1, 1, 3);
    if (!token_eq(&token, &expected)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT
            " instead.",
            TOKEN_PRINTF(expected),
            TOKEN_PRINTF(token)
        );
    }
}

void test_ints_with_whitespace(void) {
    Lexer lexer = lexer_new("  \n  123  \n  456");
    const Token token1 = lexer_pop(&lexer);
    const Token expected1 = NEW_TOKEN(INT, 5, 2, 3, 3);
    if (!token_eq(&token1, &expected1)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT
            " instead.",
            TOKEN_PRINTF(expected1),
            TOKEN_PRINTF(token1)
        );
    }
    const Token token2 = lexer_pop(&lexer);
    const Token expected2 = NEW_TOKEN(INT, 13, 3, 3, 3);
    if (!token_eq(&token2, &expected2)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT
            " instead.",
            TOKEN_PRINTF(expected2),
            TOKEN_PRINTF(token2)
        );
    }
}

void test_ints_with_words(void) {
    const char* text = " \n123abc123";
    Lexer lexer = lexer_new(text);
    Token expected[] = {
        NEW_TOKEN(INT, 2, 2, 1, 3),
        NEW_TOKEN(WORD, 5, 2, 4, 6),
        NEW_TOKEN(EOF, 11, 2, 10, 0),
    };
    for (size_t i = 0; i < ARRAY_LEN(expected); i++) {
        const Token token = lexer_pop(&lexer);
        EXPECT_TOKEN_EQ(token, expected[i]);
    }
}

void test_symbols(void) {
    const char* text = "+-*/()";
    Lexer lexer = lexer_new(text);
    Token expected[] = {
        NEW_TOKEN(PLUS, 0, 1, 1, 1),   NEW_TOKEN(MINUS, 1, 1, 2, 1),
        NEW_TOKEN(STAR, 2, 1, 3, 1),   NEW_TOKEN(SLASH, 3, 1, 4, 1),
        NEW_TOKEN(LPAREN, 4, 1, 5, 1), NEW_TOKEN(RPAREN, 5, 1, 6, 1),
        NEW_TOKEN(EOF, 6, 1, 7, 0),
    };
    for (size_t i = 0; i < ARRAY_LEN(expected); i++) {
        const Token token = lexer_pop(&lexer);
        EXPECT_TOKEN_EQ(token, expected[i]);
    }
}

void test_brackets(void) {
    const char* text = "[]{}";
    Lexer l = lexer_new(text);
    const Token expected[] = {
        NEW_TOKEN(LSQUARE, 0, 1, 1, 1),
        NEW_TOKEN(RSQUARE, 1, 1, 2, 1),
        NEW_TOKEN(LCURLY, 2, 1, 3, 1),
        NEW_TOKEN(RCURLY, 3, 1, 4, 1),
    };
    for (size_t i = 0; i < ARRAY_LEN(expected); i++) {
        const Token token = lexer_pop(&l);
        EXPECT_TOKEN_EQ(token, expected[i]);
    }
    lexer_free(&l);
}

int main(void) {
    test_eof();
    test_single_int();
    test_ints_with_whitespace();
    test_ints_with_words();
    test_symbols();
    test_brackets();
    return 0;
}
