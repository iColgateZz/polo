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
    if (parser.current + i >= parser.tokens.count)
        return parser.tokens.items[parser.tokens.count - 1];

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
AstNode *_error(byte *expected) {
    Token t = _peek();
    fprintf(stderr, "Parse error: expected '%s' but got '%.*s' at line %d\n", 
        expected, (i32) t.str.len, t.str.s, t.line);
    parser.error = true;
    parser.panic = true;
    return new_error_node(t, expected);
}

static inline
b32 _is_type(Token t) {
    return t.type == TOKEN_NUM ||
           t.type == TOKEN_STRING ||
           t.type == TOKEN_BOOL ||
           t.type == TOKEN_VOID;
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
static AstNode *parse_declaration(void);
static AstNode *parse_fun_decl(void);
static AstNode *parse_parameters(void);
static AstNode *parse_block(void);
static AstNode *parse_call(void);
static AstNode *parse_arguments(void);
static AstNode *parse_statement(void);
static AstNode *parse_return_stmt(void);
static AstNode *parse_print_stmt(void);
static AstNode *parse_while_stmt(void);
static AstNode *parse_assignment_stmt(void);

static inline
void _synchronize(void) {
    while (!_at_end()) {
        TokenType t = _peek().type;
        if (t == TOKEN_SEMICOLON || t == TOKEN_RIGHT_BRACE) {
            _advance(); // Go past the semicolon
            break;
        }

        if (t == TOKEN_LEFT_BRACE) {
            parse_block();
            return;
        }

        _advance();
    }
}

static 
AstNode *parse_program(void) {
    AstNodeArray decls = {0};
    while (!_at_end()) {
        AstNode *decl = parse_declaration();
        da_append(&decls, decl);

        if (parser.panic) {
            parser.panic = false;
            _synchronize();
        }
    }
    return new_program_node(decls);
}

static AstNode *parse_declaration(void) {
    Token lookahead = _look(2);
    
    if (lookahead.type == TOKEN_LEFT_PAREN)
        return parse_fun_decl();

    return parse_var_decl();
}

static AstNode *parse_fun_decl(void) {
    AstNode *type = parse_type();
    no_panic(type);

    Token name = _peek();
    if (!_match(TOKEN_IDENTIFIER_LITERAL))
        return _error("function name");

    if (!_match(TOKEN_LEFT_PAREN))
        return _error("(");

    AstNode *params = parse_parameters();
    no_panic(params);

    if (!_match(TOKEN_RIGHT_PAREN))
        return _error("')'");

    if (_peek().type == TOKEN_LEFT_BRACE) {
        AstNode *body = parse_block();
        no_panic(body);

        return new_function_decl_node(type, name, params, body);
    } 

    if (_match(TOKEN_SEMICOLON))
        return new_function_decl_node(type, name, params, NULL);

    return _error("function body or ';'");
}

static 
AstNode *parse_parameters(void) {
    AstNodeArray params = {0};
    if (_peek().type == TOKEN_RIGHT_PAREN)
        return new_parameter_list_node(params);

    do {
        AstNode *type = parse_type();
        no_panic(type);

        Token name = _peek();
        if (!_match(TOKEN_IDENTIFIER_LITERAL))
            return _error("parameter name");

        AstNode *param = new_parameter_node(type, name);
        da_append(&params, param);
    } while (_match(TOKEN_COMMA));

    return new_parameter_list_node(params);
}

static 
AstNode *parse_block(void) {
    _match(TOKEN_LEFT_BRACE);
    AstNodeArray stmts = {0};

    while (_peek().type != TOKEN_RIGHT_BRACE && !_at_end()) {
        AstNode *stmt;
        if (_is_type(_peek())) {
            stmt = parse_var_decl();
        } else {
            stmt = parse_statement();
        }
        
        if (parser.panic) {
            parser.panic = false;
            _synchronize();
        }

        da_append(&stmts, stmt);
    }

    if (!_match(TOKEN_RIGHT_BRACE))
            return _error("}");

    return new_block_node(stmts);
}

static 
AstNode *parse_statement(void) {
    Token t = _peek();
    switch (t.type) {
        case TOKEN_RETURN:
            return parse_return_stmt();
        case TOKEN_LEFT_BRACE:
            return parse_block();
        case TOKEN_PRINT: 
            return parse_print_stmt();
        case TOKEN_WHILE:
            return parse_while_stmt();
        case TOKEN_IDENTIFIER_LITERAL: {
            if (_look(1).type == TOKEN_LEFT_PAREN)
                break;
            return parse_assignment_stmt();
        }
        // case TOKEN_IF: return parse_if_stmt();
        // case TOKEN_FOR: return parse_for_stmt();
        // case TOKEN_BREAK: return parse_break_stmt();
        // case TOKEN_CONTINUE: return parse_continue_stmt();
        default: break;
    }
    AstNode *expr = parse_expression();
    no_panic(expr);

    if (!_match(TOKEN_SEMICOLON))
        return _error("';'");

    return new_expr_stmt_node(expr);
}

static 
AstNode *parse_return_stmt(void) {
    _match(TOKEN_RETURN);
    AstNode *expr = parse_expression();
    no_panic(expr);

    if (!_match(TOKEN_SEMICOLON))
        return _error(";");

    return new_return_stmt_node(expr);
}

static 
AstNode *parse_print_stmt(void) {
    _match(TOKEN_PRINT);
    AstNode *expr = parse_expression();
    no_panic(expr);

    if (!_match(TOKEN_SEMICOLON))
        return _error(";");

    return new_print_stmt_node(expr);
}

static 
AstNode *parse_while_stmt(void) {
    _match(TOKEN_WHILE);

    if (!_match(TOKEN_LEFT_PAREN))
        return _error("(");

    AstNode *condition = parse_expression();
    no_panic(condition);

    if (!_match(TOKEN_RIGHT_PAREN))
        return _error(")");

    AstNode *body = parse_block();
    no_panic(condition);

    return new_while_stmt_node(condition, body);
}

static
AstNode *parse_assignment_stmt(void) {
    Token t = _peek();
    _match(TOKEN_IDENTIFIER_LITERAL);
    AstNode *lvalue = new_identifier_node(t);
    no_panic(lvalue);

    if (!_match(TOKEN_EQUAL)) 
        return _error("=");

    AstNode *rvalue = parse_assignment();
    no_panic(rvalue);

    if (!_match(TOKEN_SEMICOLON))
        return _error(";");
    
    return new_assign_stmt_node(lvalue, rvalue);
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
        _match(TOKEN_BOOL) ||
        _match(TOKEN_VOID)) {
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

static 
AstNode *parse_unary(void) {
    if (_match(TOKEN_BANG) || _match(TOKEN_MINUS)) {
        Token op = _look(-1);
        AstNode *operand = parse_unary();
        no_panic(operand);
        return new_unary_expr_node(operand, op);
    }
    return parse_call();
}

static 
AstNode *parse_call(void) {
    AstNode *expr = parse_primary();
    no_panic(expr);

    if (_match(TOKEN_LEFT_PAREN)) {
        AstNode *args = parse_arguments();
        no_panic(args);

        if (!_match(TOKEN_RIGHT_PAREN))
            return _error("')' after arguments");

        return new_call_expr_node(expr, args);
    }

    return expr;
}

static 
AstNode *parse_arguments(void) {
    AstNodeArray args = {0};
    if (_peek().type == TOKEN_RIGHT_PAREN)
        return new_argument_list_node(args);

    do {
        AstNode *arg = parse_expression();
        no_panic(arg);
        da_append(&args, arg);
    } while (_match(TOKEN_COMMA));
    
    return new_argument_list_node(args);
}

static 
AstNode *parse_primary(void) {
    Token t = _peek();

    if (_match(TOKEN_NUMBER_LITERAL))
        return new_number_literal_node(t);

    if (_match(TOKEN_STRING_LITERAL))
        return new_string_literal_node(t);

    if (_match(TOKEN_BOOL_LITERAL))
        return new_bool_literal_node(t);

    if (_match(TOKEN_IDENTIFIER_LITERAL))
        return new_identifier_node(t);

    if (_match(TOKEN_LEFT_PAREN)) {
        AstNode *expr = parse_expression();
        no_panic(expr);

        if (!_match(TOKEN_RIGHT_PAREN))
            return _error(")");

        return new_paren_expr_node(expr);
    }
    
    return _error("primary");
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
