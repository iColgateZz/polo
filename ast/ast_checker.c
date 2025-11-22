#include "ast_checker.h"
#include "special_nodes.h"
#include "token.h"
#include <stdio.h>
#include "da.h"
#include <stdarg.h>

typedef struct {
    b32 error;
    b32 panic;
} Checker;

static Checker checker;

void _init_checker(void) {
    checker = (Checker) {0};
}

#define no_panic(retval) do { if (checker.panic) return retval; } while (0)

static void _semantic_error(const byte *fmt, ...) {
    fprintf(stderr, "Semantic error: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    checker.error = true;
    checker.panic = true;
}

typedef struct {
    Token name;
    AstNode *type;
} Symbol;

typedef struct {
    Symbol *items;
    size_t count;
    size_t capacity;
} SymbolTable;

static SymbolTable global_symbols;

void _init_global(void) {
    global_symbols = (SymbolTable) {0};
}

AstNode *_lookup_global(Token name) {
    for (size_t i = 0; i < global_symbols.count; ++i) {
        if (global_symbols.items[i].name.str.len == name.str.len &&
            memcmp(global_symbols.items[i].name.str.s, name.str.s, name.str.len) == 0) {
            return global_symbols.items[i].type;
        }
    }
    return NULL;
}

void _add_global(Token name, AstNode *type) {
    Symbol s = {.name = name, .type = type};
    da_append(&global_symbols, s);
}

static PrimitiveTypeNode sentinel_type;
AstNode *_get_type_of(AstNode *node) {
    switch (node->ast_type) {
        case AST_LITERAL_NUMBER:
            sentinel_type.this.ast_type = AST_TYPE_NUM;
            return (AstNode *)&sentinel_type;
        case AST_LITERAL_STRING:
            sentinel_type.this.ast_type = AST_TYPE_STRING;
            return (AstNode *)&sentinel_type;
        case AST_LITERAL_BOOL:
            sentinel_type.this.ast_type = AST_TYPE_BOOL;
            return (AstNode *)&sentinel_type;
        default:
            return node; // For identifiers, you should already be returning their type
    }
}

b32 _types_compatible(AstNode *lhs_type, AstNode *rhs_type) {
    if (!lhs_type || !rhs_type) return false;
    return _get_type_of(lhs_type)->ast_type == _get_type_of(rhs_type)->ast_type;
}

AstNode *_check_node(AstNode *node) {
    no_panic(NULL);
    if (!node) return NULL;

    switch (node->ast_type) {
        case AST_PROGRAM: {
            ProgramNode *prog = (ProgramNode *)node;
            for (usize i = 0; i < prog->declarations.count; ++i) {
                _check_node(prog->declarations.items[i]);
                checker.panic = false; // Reset panic for next declaration
            }
            return NULL;
        }

        case AST_VAR_DECL: {
            VarDeclNode *var = (VarDeclNode *)node;
            if (_lookup_global(var->name)) {
                _semantic_error("redeclaration of variable '%.*s' at line %d",
                (i32)var->name.str.len, var->name.str.s, var->name.line);
                return NULL;
            }

            _add_global(var->name, var->type);
            if (var->initializer) {
                AstNode *init_type = _check_node(var->initializer);
                no_panic(NULL);
                if (!_types_compatible(var->type, init_type)) {
                    _semantic_error("type mismatch in assignment to '%.*s' at line %d",
                        (i32)var->name.str.len, var->name.str.s, var->name.line);
                }
            }
            return NULL;
        }

        case AST_LITERAL_NUMBER:
        case AST_LITERAL_STRING:
        case AST_LITERAL_BOOL:
        case AST_LITERAL_NULL:
        case AST_TYPE_NUM:
        case AST_TYPE_STRING:
        case AST_TYPE_BOOL:
        case AST_TYPE_VOID:
        case AST_TYPE_STRUCT:
            return node;

        case AST_IDENTIFIER: {
            IdentifierNode *id = (IdentifierNode *)node;
            AstNode *type = _lookup_global(id->name);
            if (!type) {
                _semantic_error("use of undefined variable '%.*s' at line %d",
                    (i32)id->name.str.len, id->name.str.s, id->name.line);
                return NULL;
            }
            return type;
        }

        case AST_ASSIGN_EXPR: {
            AssignExprNode *assign = (AssignExprNode *)node;
            AstNode *lhs_type = _check_node(assign->lvalue);
            no_panic(NULL);
            AstNode *rhs_type = _check_node(assign->value);
            no_panic(NULL);
            if (!_types_compatible(lhs_type, rhs_type)) {
                _semantic_error("type mismatch in assignment at line %d",
                    assign->lvalue ? ((IdentifierNode *)assign->lvalue)->name.line : 0);
                return NULL;
            }
            return lhs_type;
        }

        case AST_BINARY_EXPR: {
            BinaryExprNode *bin = (BinaryExprNode *)node;
            AstNode *left_type = _check_node(bin->left);
            no_panic(NULL);
            AstNode *right_type = _check_node(bin->right);
            no_panic(NULL);
            if (!_types_compatible(left_type, right_type)) {
                _semantic_error("type mismatch in binary expression '%.*s' at line %d",
                    (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                return NULL;
            }
            return left_type;
        }

        case AST_UNARY_EXPR: {
            UnaryExprNode *un = (UnaryExprNode *)node;
            AstNode *operand_type = _check_node(un->operand);
            no_panic(NULL);
            return operand_type;
        }

        case AST_PAREN_EXPR: {
            ParenExprNode *paren = (ParenExprNode *)node;
            AstNode *expr_type = _check_node(paren->expression);
            no_panic(NULL);
            return expr_type;
        }

        default:
            _semantic_error("unknown node type");
            return NULL;
    }
}

b32 semantic_errors(AstNode *program) {
    // Assignment eval from the right
    // Can't assign to non-variable object
    // Figure out what to do with ! and - in unary
    _init_checker();
    _init_global();

    _check_node(program);

    return checker.error;
}