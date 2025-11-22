#include "parser.h"
#include "special_nodes.h"
#include <stdlib.h>
#include <stdio.h>
#include "da.h"

typedef struct {
    TokenArray tokens;
    size_t current;
    b32 error;
    b32 panic;
} Parser;

static Parser parser;

static inline 
Token _peek(void) {
    return parser.tokens.items[parser.current];
}

static inline
Token _look(i32 i) {
    return parser.tokens.items[parser.current + i];
}

static inline 
b32 _at_end(void) {
    return _peek().type == TOKEN_EOF;
}

static inline 
Token _advance(void) {
    if (!_at_end()) ++parser.current;
    return _peek();
}

static inline 
b32 _match(TokenType type) {
    if (_peek().type == type) {
        _advance();
        return true;
    }
    return false;
}

static inline
void _synchronize(void) {
    // Skip tokens until we find a semicolon or EOF
    while (!_at_end()) {
        if (_peek().type == TOKEN_SEMICOLON) {
            _advance(); // Go past the semicolon
            break;
        }
        _advance();
    }
}

static inline 
AstNode *_error(byte *expected) {
    Token t = _peek();
    fprintf(stderr, "Parse error: expected '%s' but got '%.*s' at line %d\n", 
        expected, (i32) t.str.len, t.str.s, t.line);
    parser.error = true;
    parser.panic = true;
    return new_error_node(t, expected);
}

#define no_panic(node) do { if (parser.panic) return node; } while (0)

static AstNode *parse_var_decl(void);
static AstNode *parse_type(void);
static AstNode *parse_expression(void);
static AstNode *parse_assignment(void);
static AstNode *parse_logic_or(void);
static AstNode *parse_logic_and(void);
static AstNode *parse_equality(void);
static AstNode *parse_comparison(void);
static AstNode *parse_term(void);
static AstNode *parse_factor(void);
static AstNode *parse_unary(void);
static AstNode *parse_primary(void);

static 
AstNode *parse_program(void) {
    AstNodeArray decls = {0};
    while (!_at_end()) {
        AstNode *decl = parse_var_decl();
        da_append(&decls, decl);

        if (parser.panic) {
            parser.panic = false;
            _synchronize();
        }
    }
    return new_program_node(decls);
}

static 
AstNode *parse_var_decl(void) {
    AstNode *type = parse_type();
    no_panic(type);

    Token name = _peek();
    if (!_match(TOKEN_IDENTIFIER_LITERAL)) {
        return _error("var_name");
    }

    AstNode *initializer = NULL;
    if (_match(TOKEN_EQUAL)) {
        initializer = parse_expression();
        no_panic(initializer);
    }

    if (!_match(TOKEN_SEMICOLON)) {
        return _error(";");
    }
    return new_var_decl_node(type, name, initializer);
}

static 
AstNode *parse_type(void) {
    Token t = _peek();
    if (_match(TOKEN_NUM) ||
        _match(TOKEN_STRING) ||
        _match(TOKEN_BOOL)) {
        return new_primitive_type_node(t);
    } else {
        return _error("type");
    }
}

static 
AstNode *parse_expression(void) {
    return parse_assignment();
}

static 
AstNode *parse_assignment(void) {
    AstNode *left = parse_logic_or();
    no_panic(left);

    if (_match(TOKEN_EQUAL)) {
        AstNode *value = parse_assignment();
        no_panic(value);
        // Only lvalues can be assigned to; for now, just wrap as assign expr
        return new_assign_expr_node(left, value);
    }

    return left;
}

static 
AstNode *parse_logic_or(void) {
    AstNode *left = parse_logic_and();
    no_panic(left);

    while (_match(TOKEN_OR)) {
        Token op = _look(-1);
        AstNode *right = parse_logic_and();
        no_panic(right);
        left = new_binary_expr_node(left, right, op);
    }

    return left;
}

static 
AstNode *parse_logic_and(void) {
    AstNode *left = parse_equality();
    no_panic(left);

    while (_match(TOKEN_AND)) {
        Token op = _look(-1);
        AstNode *right = parse_equality();
        no_panic(right);
        left = new_binary_expr_node(left, right, op);
    }

    return left;
}

static 
AstNode *parse_equality(void) {
    AstNode *left = parse_comparison();
    no_panic(left);

    while (_match(TOKEN_BANG_EQUAL) || _match(TOKEN_EQUAL_EQUAL)) {
        Token op = _look(-1);
        AstNode *right = parse_comparison();
        no_panic(right);
        left = new_binary_expr_node(left, right, op);
    }

    return left;
}

static 
AstNode *parse_comparison(void) {
    AstNode *left = parse_term();
    no_panic(left);

    while (_match(TOKEN_GREATER) || _match(TOKEN_GREATER_EQUAL) ||
           _match(TOKEN_LESS) || _match(TOKEN_LESS_EQUAL)) {
        Token op = _look(-1);
        AstNode *right = parse_term();
        no_panic(right);
        left = new_binary_expr_node(left, right, op);
    }

    return left;
}

static 
AstNode *parse_term(void) {
    AstNode *left = parse_factor();
    no_panic(left);

    while (_match(TOKEN_MINUS) || _match(TOKEN_PLUS)) {
        Token op = _look(-1);
        AstNode *right = parse_factor();
        no_panic(right);
        left = new_binary_expr_node(left, right, op);
    }

    return left;
}

static 
AstNode *parse_factor(void) {
    AstNode *left = parse_unary();
    no_panic(left);

    while (_match(TOKEN_SLASH) || _match(TOKEN_STAR)) {
        Token op = _look(-1);
        AstNode *right = parse_unary();
        no_panic(right);
        left = new_binary_expr_node(left, right, op);
    }

    return left;
}

static AstNode *parse_unary(void) {
    if (_match(TOKEN_BANG) || _match(TOKEN_MINUS)) {
        Token op = _look(-1);
        AstNode *operand = parse_unary();
        no_panic(operand);
        return new_unary_expr_node(operand, op);
    }
    return parse_primary();
}

static AstNode *parse_primary(void) {
    Token t = _peek();

    if (_match(TOKEN_NUMBER_LITERAL)) {
        return new_number_literal_node(t);
    } else if (_match(TOKEN_STRING_LITERAL)) {
        return new_string_literal_node(t);
    } else if (_match(TOKEN_BOOL_LITERAL)) {
        return new_bool_literal_node(t);
    } else if (_match(TOKEN_NULL_LITERAL)) {
        return new_null_literal_node(t);
    } else if (_match(TOKEN_IDENTIFIER_LITERAL)) {
        return new_identifier_node(t);
    } else if (_match(TOKEN_LEFT_PAREN)) {
        AstNode *expr = parse_expression();
        no_panic(expr);

        if (!_match(TOKEN_RIGHT_PAREN)) {
            return _error(")");
        }
        return new_paren_expr_node(expr);
    } else {
        return _error("primary");
    }
}

static inline
void _init_parser(TokenArray tokens) {
    parser.tokens = tokens;
    parser.current = 0;
    parser.error = false;
    parser.panic = false;
}

ParseResult parse(TokenArray tokens) {
    _init_parser(tokens);
    init_special_nodes();
    return (ParseResult) { parse_program(), parser.error };
}

void free_ast(AstNode *program) {
    // free da arrays
    // Later when there are more nodes with da arrays
    // I should probably store refs to the arrays 
    // to simplify freeing. 
    ProgramNode *p = (ProgramNode *)program;
    free(p->declarations.items);
    free_special_nodes();
}
