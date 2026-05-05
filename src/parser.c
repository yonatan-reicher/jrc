#include "parser.h"
#include "basic.h"
#include "slice.h"
#include "str.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

Parser parser_new(Token (*get_token)(void* ctx), void* get_token_ctx) {
    Parser parser = {
        .get_token = get_token,
        .get_token_ctx = get_token_ctx,
        .has_peeked_token = false,
        .peeked_token = { 0 },
        .arena = NULL,
        .arena_size = 0,
        .arena_capacity = 0,
        .previous_arenas = NULL,
        .n_previous_arenas = 0,
    };
    return parser;
}

void parser_free(Parser* p) {
    for (size_t i = 0; i < p->n_previous_arenas; i++) {
        free(p->previous_arenas[i]);
    }
    free(p->arena);
    *p = (Parser) { 0 };
}

void* alloc(Parser* p, size_t size) {
    // Allocate a new arena if the current one is done.
    if (p->arena_size + size > p->arena_capacity) {
        size_t new_capacity = (p->arena_size + size) * 2;
        void* new_arena = malloc(new_capacity);
        p->previous_arenas =
            realloc(p->previous_arenas, ++p->n_previous_arenas * sizeof(void*));
        p->previous_arenas[p->n_previous_arenas - 1] = p->arena;
        p->arena = new_arena;
        p->arena_capacity = new_capacity;
        p->arena_size = 0;
    }
    // Get the address of the new allocation.
    void* ptr = (void*)((char*)p->arena + p->arena_size);
    // Update the size. Make sure to keep alignment of `size_t`.
    p->arena_size += (size + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);
    return ptr;
}

#define ALLOC(p, type) (type*)alloc(p, sizeof(type))

Token get_token(Parser* p) {
    if (p->has_peeked_token) {
        p->has_peeked_token = false;
        return p->peeked_token;
    } else {
        return p->get_token(p->get_token_ctx);
    }
}

static Token peek(Parser* p) {
    if (!p->has_peeked_token) {
        p->peeked_token = p->get_token(p->get_token_ctx);
        p->has_peeked_token = true;
    }
    return p->peeked_token;
}

static AstError* err(Parser* p, TextSpan span, const char* message) {
    size_t message_len = strlen(message);
    AstError* ast = alloc(p, sizeof(AstError) + message_len + 1);
    ast->art.kind = AST_ERROR;
    ast->art.span = span;
    memcpy(ast->message, message, message_len + 1);
    return ast;
}

static Ast* parse_int(Parser* p, Token t) {
#define MSG_INVALID "Invalid integer literal"
#define MSG_RANGE "Integer literal is too big or too small"
#define MSG_UNKNOWN "Unknown error while parsing integer literal"
    char* end;
    intmax_t i = strtoimax(t.text, &end, 0);
    if (errno) {
        switch (errno) {
            case EINVAL: return (Ast*)err(p, t.span, MSG_INVALID);
            case ERANGE: return (Ast*)err(p, t.span, MSG_RANGE);
            default: return (Ast*)err(p, t.span, MSG_UNKNOWN);
        }
    } else if (end != t.text + token_len(&t)) {
        return (Ast*)err(p, t.span, MSG_INVALID);
    } else if (i < INT64_MIN || i > INT64_MAX) {
        return (Ast*)err(p, t.span, MSG_RANGE);
    }
    AstInt* ast = ALLOC(p, AstInt);
    ast->ast.kind = AST_INT;
    ast->ast.span = t.span;
    ast->value = (int64_t)i;
    return (Ast*)ast;
}

static Ast* parse_var(Parser* p, Token t) {
    size_t name_len = token_len(&t);
    AstVar* ast = alloc(p, sizeof(AstVar) + name_len + 1);
    *ast = (AstVar) { { AST_VAR, t.span } };
    memcpy(ast->name, t.text, name_len);
    ast->name[name_len] = '\0';
    return (Ast*)ast;
}

static Ast* parse_expr(Parser*);

static Ast* parse_atom(Parser* p) {
    const Token t = get_token(p);
    switch (t.kind) {
        case TOKEN_KIND_INT: return parse_int(p, t);
        case TOKEN_KIND_WORD: return parse_var(p, t);
        case TOKEN_KIND_LPAREN: {
            Ast* ast = parse_expr(p);
            const Token next = get_token(p);
            if (next.kind != TOKEN_KIND_RPAREN) {
                return (Ast*)err(p, next.span, "Expected ')'");
            }
            return ast;
        }
        default: return (Ast*)err(p, t.span, "Unexpected token");
    }
}

static bool try_parse_unary_op(Parser* p, UnaryOp* out_op) {
    const Token t = peek(p);
    switch (t.kind) {
        case TOKEN_KIND_PLUS: *out_op = UNARY_OP_POS; goto single_token_success;
        case TOKEN_KIND_MINUS:
            *out_op = UNARY_OP_NEG;
            goto single_token_success;
        default: return false;
    }
single_token_success:
    get_token(p);
    return true;
}

static Ast* parse_term(Parser* p) {
    const Token t = peek(p);
    UnaryOp op;
    if (try_parse_unary_op(p, &op)) {
        Ast* arg = parse_term(p);
        AstUnaryOp* ast = ALLOC(p, AstUnaryOp);
        *ast = (AstUnaryOp) {
            { AST_UNARY_OP, { t.span.start, arg->span.end } },
            op,
            arg,
        };
        return (Ast*)ast;
    } else {
        return parse_atom(p);
    }
}

bool try_parse_bin_op(const Token* t, BinOp* out_op) {
#define RETURN(SUCCESS, OP)                                                    \
    do {                                                                       \
        *out_op = (OP);                                                        \
        return (SUCCESS);                                                      \
    } while (0)
    switch (t->kind) {
        case TOKEN_KIND_PLUS: RETURN(true, BIN_OP_ADD);
        case TOKEN_KIND_MINUS: RETURN(true, BIN_OP_SUB);
        case TOKEN_KIND_STAR: RETURN(true, BIN_OP_MUL);
        case TOKEN_KIND_SLASH: RETURN(true, BIN_OP_DIV);
        case TOKEN_KIND_PERCENT: RETURN(true, BIN_OP_REM);
        case TOKEN_KIND_NULL:
        case TOKEN_KIND_ERROR:
        case TOKEN_KIND_EOF:
        case TOKEN_KIND_INT:
        case TOKEN_KIND_WORD:
        case TOKEN_KIND_STR:
        case TOKEN_KIND_LPAREN:
        case TOKEN_KIND_RPAREN:
        case TOKEN_KIND_COLON_EQ:
        case TOKEN_KIND_COLON:
        case TOKEN_KIND_SEMICOLON:
        case TOKEN_KIND_LSQUARE:
        case TOKEN_KIND_RSQUARE:
        case TOKEN_KIND_LCURLY:
        case TOKEN_KIND_RCURLY: RETURN(false, 0);
    }
#undef RETURN
}

bool try_parse_bin_op_min_precedence(
    const Token* t, BinOp* out_op, BinOpPrecedence min_prec
) {
    bool success = try_parse_bin_op(t, out_op);
    return success && bin_op_precedence(*out_op) >= min_prec;
}

static Ast* parse_bin_op_expr_with_min_precedence(
    Parser* p, Ast* lhs, BinOpPrecedence min_prec
) {
    Token t = peek(p);
    BinOp next_op;
    while (try_parse_bin_op_min_precedence(&t, &next_op, min_prec)) {
        BinOp op = next_op;
        BinOpPrecedence op_prec = bin_op_precedence(op);
        get_token(p);
        Ast* rhs = parse_term(p);
        t = peek(p);
        while (try_parse_bin_op_min_precedence(&t, &next_op, op_prec + 1) ||
               (try_parse_bin_op_min_precedence(&t, &next_op, op_prec) &&
                bin_op_is_right_associative(next_op))) {
            const BinOpPrecedence next_op_prec = bin_op_precedence(next_op);
            rhs = parse_bin_op_expr_with_min_precedence(
                p, rhs, op_prec + (next_op_prec > op_prec ? 1 : 0)
            );
            t = peek(p);
        }
        AstBinOp* new_left = ALLOC(p, AstBinOp);
        *new_left = (AstBinOp) {
            { AST_BIN_OP, { lhs->span.start, rhs->span.end } },
            op,
            lhs,
            rhs,
        };
        lhs = (Ast*)new_left;
    }
    return lhs;

    // Taken from Wikipedia
    //
    // parse_expression()
    //     return parse_expression_1(parse_primary(), 0)
    //
    // parse_expression_1(lhs, min_precedence)
    //     lookahead := peek next token
    //     while lookahead is a binary operator whose precedence is >=
    //     min_precedence
    //         op := lookahead
    //         advance to next token
    //         rhs := parse_primary ()
    //         lookahead := peek next token
    //         while lookahead is a binary operator whose precedence is greater
    //                  than op's, or a right-associative operator
    //                  whose precedence is equal to op's
    //             rhs := parse_expression_1 (rhs, precedence of op + (1 if
    //             lookahead precedence is greater, else 0)) lookahead := peek
    //             next token
    //         lhs := the result of applying op with operands lhs and rhs
    //     return lhs
    //
}

static Ast* parse_bin_op_expr(Parser* p) {
    return parse_bin_op_expr_with_min_precedence(p, parse_term(p), 0);
}

static Ast* parse_expr(Parser* p) {
    return parse_bin_op_expr(p);
}

static Ast* parser_assign(Parser* p) {
    const Token var = get_token(p);
    if (var.kind != TOKEN_KIND_WORD) {
        PANIC("this should never happen!");
    }
    const Token op = get_token(p);
    if (op.kind != TOKEN_KIND_COLON_EQ) {
        return (Ast*)err(p, op.span, "expected a ':=' here");
    }
    Ast* rhs = parse_expr(p);
    const Token end = get_token(p);
    if (end.kind != TOKEN_KIND_SEMICOLON) {
        free(rhs);
        return (Ast*)err(p, end.span, "expected ';' here");
    }
    AstAssign* ret = alloc(p, sizeof(AstAssign) + token_len(&var) + 1);
    *ret = (AstAssign) {
        { AST_ASSIGN, { var.span.start, rhs->span.end } },
        rhs,
    };
    ConstCharSlice var_slice = token_slice(&var);
    memcpy(ret->var, var_slice.ptr, var_slice.len);
    return (Ast*)ret;
}

Ast* parser_parse_statement(Parser* p) {
    const Token t = peek(p);
    switch (t.kind) {
        case TOKEN_KIND_WORD: return parser_assign(p);
        default: return (Ast*)err(p, t.span, "not start of a statement");
    }
}

Ast* parser_parse(Parser* p) {
    return parse_expr(p);
}
