#include "parser.h"
#include "array.h"
#include "basic.h"
#include "slice.h"
#include "str.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

Parser parser_new(Token (*get_token)(void* ctx), void* get_token_ctx) {
    Parser parser = {
        get_token, get_token_ctx, 0, {}, NULL, 0, 0, NULL, 0, false,
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
    switch (p->n_tokens_peeked) {
        case 0: return p->get_token(p->get_token_ctx);
        case 1: return (p->n_tokens_peeked = 0, p->peeked_tokens[0]);
        case 2:
            Token ret = p->peeked_tokens[0];
            p->peeked_tokens[0] = p->peeked_tokens[1];
            p->n_tokens_peeked = 1;
            return ret;
        default: PANIC("this should not happen");
    }
}

static Token peek(Parser* p) {
    if (p->n_tokens_peeked == 0) {
        p->peeked_tokens[0] = p->get_token(p->get_token_ctx);
        p->n_tokens_peeked = 1;
    }
    return p->peeked_tokens[0];
}

/// Peek the token after the next one
static Token peek_second(Parser* p) {
    if (p->n_tokens_peeked == 0) {
        p->peeked_tokens[0] = p->get_token(p->get_token_ctx);
        p->peeked_tokens[1] = p->get_token(p->get_token_ctx);
        p->n_tokens_peeked = 2;
    } else if (p->n_tokens_peeked == 1) {
        p->peeked_tokens[1] = p->get_token(p->get_token_ctx);
        p->n_tokens_peeked = 2;
    }
    return p->peeked_tokens[1];
}

#define err(P, SPAN, ...) err_((P), (SPAN), str_format(__VA_ARGS__))

static AstError* err_(Parser* p, TextSpan span, char* message) {
    size_t message_len = strlen(message);
    AstError* ast = alloc(p, sizeof(AstError) + message_len + 1);
    *ast = (AstError) { { AST_ERROR, span } };
    memcpy(ast->message, message, message_len + 1);
    free(message);
    p->had_err = true;
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
    *ast = (AstInt) {
        { AST_INT, t.span },
        (int64_t)i,
    };
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

/// This function assumes that the parameter name already was parsed and we are
/// at the `=>`.
static Ast* parse_func(Parser* p, Token param_name) {
    const Token fat_arrow = get_token(p);
    EXPECT(fat_arrow.kind == TOKEN_KIND_FAT_ARROW, "Expected '=>'");
    Ast* body = parse_expr(p);
    AstFunc* ast = alloc(p, sizeof(AstFunc) + token_len(&param_name) + 1);
    *ast = (AstFunc) {
        { AST_FUNC, { param_name.span.start, body->span.end } },
        body,
    };
    memcpy(ast->param_name, param_name.text, token_len(&param_name));
    ast->param_name[token_len(&param_name)] = '\0';
    return (Ast*)ast;
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
        if (peek(p).kind == TOKEN_KIND_WORD) {
            // Now this is either a variable or a function.
            const Token param_name = get_token(p);
            return peek(p).kind == TOKEN_KIND_FAT_ARROW
                       ? parse_func(p, param_name)
                       : parse_var(p, t);
        }
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
        default: RETURN(false, (BinOp)0);
    }
#undef RETURN
    PANIC("invalid token kind %d", t->kind);
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
                p,
                rhs,
                (BinOpPrecedence)(op_prec + (next_op_prec > op_prec ? 1 : 0))
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

static Ast* parse_assign(Parser* p) {
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

DECLARE_ARRAY(Ast*, AstPtrArray);

static Ast* parse_statement(Parser* p);

static Ast* parse_compound_statement(Parser* p) {
    const Token open = get_token(p);
    EXPECT(open.kind == TOKEN_KIND_LCURLY, "must start with '{'");
    AstPtrArray children = array_empty();
    while (peek(p).kind != TOKEN_KIND_RCURLY) {
        Ast* child = parse_statement(p);
        array_push(&children, child);
    }
    const Token end = get_token(p); // Consume the closing '}'
    AstCompoundStatement* ast =
        alloc(p, sizeof(AstCompoundStatement) + children.len * sizeof(Ast*));
    *ast = (AstCompoundStatement) {
        { AST_COMPOUND_STATEMENT, { open.span.start, end.span.end } },
        children.len,
    };
    memcpy(ast->children, children.ptr, children.len * sizeof(Ast*));
    array_free(&children);
    return (Ast*)ast;
}

Ast* parse_empty_statement(Parser* p) {
    // Yes, this is very redundant. But, consistency!
    const Token t = get_token(p);
    EXPECT(
        t.kind == TOKEN_KIND_SEMICOLON,
        "this should only be called when the next token is a semicolon"
    );
    Ast* ast = ALLOC(p, Ast);
    *ast = (Ast) { AST_EMPTY_STATEMENT, t.span };
    return ast;
}

static Ast* parse_stmt_or_expr(Parser* p) {
    const Token t = peek(p);
    if (t.kind == TOKEN_KIND_LCURLY) return parse_compound_statement(p);
    if (t.kind == TOKEN_KIND_SEMICOLON) return parse_empty_statement(p);
    if (t.kind == TOKEN_KIND_WORD && peek_second(p).kind == TOKEN_KIND_COLON_EQ)
        return parse_assign(p);
    return parse_expr(p);
}

static Ast* parse_statement(Parser* p) {
    Ast* ast = parse_stmt_or_expr(p);
    return ast_is_stmt(ast)
               ? ast
               : (Ast*)err(p, ast->span, "this is not a statement");
}

Ast* parser_parse_statement(Parser* p) {
    Ast* ret = parse_statement(p);
    if (parser_has_more_tokens(p)) {
        const Token t = peek(p);
        return (Ast*)err(p, t.span, "unexpected left-over text: '%s'", t.text);
    }
    return ret;
}

Ast* parser_parse_program(Parser* p) {
    // Parse the statements
    AstPtrArray statements = array_empty();
    while (peek(p).kind != TOKEN_KIND_EOF) {
        Ast* stmt = parse_statement(p);
        array_push(&statements, stmt);
    }
    // Now should come the end of the file.
    const Token last_token = peek(p);
    if (last_token.kind != TOKEN_KIND_EOF) {
        array_free(&statements);
        return (Ast*)err(p, last_token.span, "expected end of file");
    }
    // Return
    const size_t size = sizeof(AstProgram) + statements.len * sizeof(Ast*);
    AstProgram* ast = alloc(p, size);
    const TextPos start = statements.len > 0 ? statements.ptr[0]->span.start
                                             : last_token.span.start;
    *ast = (AstProgram) {
        { AST_PROGRAM, { start, last_token.span.end } },
        statements.len,
    };
    memcpy(ast->statements, statements.ptr, statements.len * sizeof(Ast*));
    array_free(&statements);
    return (Ast*)ast;
}

Ast* parser_parse_expr(Parser* p) {
    Ast* ret = parse_expr(p);
    if (parser_has_more_tokens(p)) {
        const Token t = peek(p);
        return (Ast*)err(p, t.span, "unexpected left-over text: '%s'", t.text);
    }
    return ret;
}

Ast* parser_parse_repl_line(Parser* p) {
    Ast* ret = parse_stmt_or_expr(p);
    if (parser_has_more_tokens(p)) {
        const Token t = peek(p);
        return (Ast*)err(p, t.span, "unexpected left-over text: '%s'", t.text);
    }
    return ret;
}

bool parser_had_err(const Parser* p) {
    return p->had_err;
}

void parser_clear_err(Parser* p) {
    p->had_err = false;
}

bool parser_has_more_tokens(Parser* p) {
    return peek(p).kind != TOKEN_KIND_EOF;
}
