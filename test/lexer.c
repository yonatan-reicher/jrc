#include "../src/lexer.h"
#include "../src/basic.h"

bool token_eq(const Token* a, const Token* b) {
    return a->index == b->index
        && a->row == b->row
        && a->col == b->col
        && a->len == b->len
        && a->kind == b->kind;
}

#define EXPECT_TOKEN_EQ(A, B) \
    do { \
        if (!token_eq(&(A), &(B))) { \
            PANIC( \
                "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT " instead.", \
                TOKEN_PRINTF((B)), \
                TOKEN_PRINTF((A)) \
            ); \
        } \
    } while (0)

void test_eof(void) {
    Lexer lexer = lexer_new("");
    const Token token = lexer_pop(&lexer);
    const Token expected = {
        .index = 0,
        .row = 1,
        .col = 1,
        .len = 0,
        .kind = TOKEN_KIND_EOF,
    };
    if (!token_eq(&token, &expected)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT " instead.",
            TOKEN_PRINTF(expected),
            TOKEN_PRINTF(token)
        );
    }
}

void test_single_int(void) {
    Lexer lexer = lexer_new("123");
    const Token token = lexer_pop(&lexer);
    const Token expected = {
        .index = 0,
        .row = 1,
        .col = 1,
        .len = 3,
        .kind = TOKEN_KIND_INT,
    };
    if (!token_eq(&token, &expected)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT " instead.",
            TOKEN_PRINTF(expected),
            TOKEN_PRINTF(token)
        );
    }
}

void test_ints_with_whitespace(void) {
    Lexer lexer = lexer_new("  \n  123  \n  456");
    const Token token1 = lexer_pop(&lexer);
    const Token expected1 = {
        .index = 5,
        .row = 2,
        .col = 3,
        .len = 3,
        .kind = TOKEN_KIND_INT,
    };
    if (!token_eq(&token1, &expected1)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT " instead.",
            TOKEN_PRINTF(expected1),
            TOKEN_PRINTF(token1)
        );
    }
    const Token token2 = lexer_pop(&lexer);
    const Token expected2 = {
        .index = 13,
        .row = 3,
        .col = 3,
        .len = 3,
        .kind = TOKEN_KIND_INT,
    };
    if (!token_eq(&token2, &expected2)) {
        PANIC(
            "Expected " TOKEN_PRINTF_FORMAT " but got " TOKEN_PRINTF_FORMAT " instead.",
            TOKEN_PRINTF(expected2),
            TOKEN_PRINTF(token2)
        );
    }
}

void test_ints_with_words(void) {
    const char* text = " \n123abc123";
    Lexer lexer = lexer_new(text);
    Token expected[] = {
        (Token) {
            .index = 2,
            .row = 2,
            .col = 1,
            .len = 3,
            .kind = TOKEN_KIND_INT,
        },
        (Token) {
            .index = 5,
            .row = 2,
            .col = 4,
            .len = 6,
            .kind = TOKEN_KIND_WORD,
        },
        (Token) {
            .index = 11,
            .row = 2,
            .col = 10,
            .len = 0,
            .kind = TOKEN_KIND_EOF,
        },
    };
    for (size_t i = 0; i < ARRAY_LEN(expected); i++) {
        const Token token = lexer_pop(&lexer);
        EXPECT_TOKEN_EQ(token, expected[i]);
    }
}

int main(void) {
    test_eof();
    test_single_int();
    test_ints_with_whitespace();
    test_ints_with_words();
}
