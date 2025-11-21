#include "parser.h"
#include "special_nodes.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    TokenArray tokens;
    size_t current;
} Parser;

static Parser parser;

static inline
Token _peek(void) {
    return parser.tokens.items[parser.current];
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
void _expect(TokenType type, byte *msg) {
    if (!_match(type)) {
        fprintf(stderr, "Parse error: expected %s at line %d\n", msg, _peek().line);
        exit(1);
    }
}

static AstNode *parse_var_decl(void);
static AstNode *parse_type(void);
static AstNode *parse_expression(void);

static AstNode *parse_program(void) {
    AstNodeArray decls = {0};
    while (!_at_end()) {
        AstNode *decl = parse_var_decl();
        // TODO: implement ast_node_array_append(&decls, decl);
    }
    return new_program_node(decls);
}

static AstNode *parse_var_decl(void) {
    AstNode *type = parse_type();

    Token name = _peek();
    _expect(TOKEN_IDENTIFIER_LITERAL, "identifier");

    AstNode *initializer = NULL;
    if (_match(TOKEN_EQUAL)) {
        initializer = parse_expression();
    }

    _expect(TOKEN_SEMICOLON, "';'");
    return new_var_decl_node(type, name, initializer);
}

static AstNode *parse_type(void) {
    Token t = _peek();
    if (_match(TOKEN_NUM) ||
        _match(TOKEN_STRING) ||
        _match(TOKEN_BOOL)) {
        return new_primitive_type_node(t);
    } else if (_match(TOKEN_IDENTIFIER_LITERAL)) {
        return new_struct_type_node(t); // For now, treat as user type
    } else {
        fprintf(stderr, "Parse error: expected type at line %d\n", _peek().line);
        exit(1);
    }
}

static AstNode *parse_expression(void) {
    // For now, just parse a literal or identifier
    Token t = _peek();
    if (_match(TOKEN_NUMBER_LITERAL)) {
        return new_number_literal_node(t);
    } else if (_match(TOKEN_STRING_LITERAL)) {
        return new_string_literal_node(t);
    } else if (_match(TOKEN_IDENTIFIER_LITERAL)) {
        return new_identifier_node(t);
    } else {
        fprintf(stderr, "Parse error: expected expression at line %d\n", _peek().line);
        exit(1);
    }
}

AstNode *parse(TokenArray tokens) {
    parser.tokens = tokens;
    parser.current = 0;
    return parse_program();
}