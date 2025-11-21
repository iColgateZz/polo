#include "ast_checker.h"
#include "special_nodes.h"
#include "token.h"
#include <stdio.h>
#include "da.h"

typedef struct {
    b32 error;
    b32 panic;
} Checker;

static Checker checker;

#define no_panic(retval) do { if (checker.panic) return retval; } while (0)
#define no_panic_void()  do { if (checker.panic) return; } while (0)

void _init_checker(void) {
    checker = (Checker) {0};
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

AstNode *_check_expression(AstNode *node);

void _check_semantics(AstNode *node) {
    no_panic_void();

    if (!node) return;

    switch (node->ast_type) {
        case AST_PROGRAM: {
            ProgramNode *prog = (ProgramNode *)node;
            for (usize i = 0; i < prog->declarations.count; ++i) {
                _check_semantics(prog->declarations.items[i]);
                if (checker.panic) {
                    checker.panic = false; // Reset panic for next declaration
                }
            }
            break;
        }
        case AST_VAR_DECL: {
            VarDeclNode *var = (VarDeclNode *)node;
            _add_global(var->name, var->type);
            if (var->initializer) {
                AstNode *init_type = _check_expression(var->initializer);
                no_panic_void();
                if (!_types_compatible(var->type, init_type)) {
                    fprintf(stderr, "Semantic error: type mismatch in assignment to '%.*s' at line %d\n",
                        (i32)var->name.str.len, var->name.str.s, var->name.line);
                    checker.error = true;
                    checker.panic = true;
                }
            }
            break;
        }
        default:
            fprintf(stderr, "Semantic error: unreachable\n");
            checker.error = true;
            checker.panic = true;
            break;
    }
}

AstNode *_check_expression(AstNode *node) {
    no_panic(NULL);

    if (!node) return NULL;

    switch (node->ast_type) {
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
                fprintf(stderr, "Semantic error: use of undefined variable '%.*s' at line %d\n",
                    (i32)id->name.str.len, id->name.str.s, id->name.line);
                checker.error = true;
                checker.panic = true;
                return NULL;
            }
            return type;
        }

        case AST_ASSIGN_EXPR: {
            AssignExprNode *assign = (AssignExprNode *)node;
            AstNode *lhs_type = _check_expression(assign->lvalue);
            no_panic(NULL);
            AstNode *rhs_type = _check_expression(assign->value);
            no_panic(NULL);
            if (!_types_compatible(lhs_type, rhs_type)) {
                fprintf(stderr, "Semantic error: type mismatch in assignment at line %d\n",
                    assign->lvalue ? ((IdentifierNode *)assign->lvalue)->name.line : 0);
                checker.error = true;
                checker.panic = true;
                return NULL;
            }
            return lhs_type;
        }

        case AST_BINARY_EXPR: {
            BinaryExprNode *bin = (BinaryExprNode *)node;
            AstNode *left_type = _check_expression(bin->left);
            no_panic(NULL);
            AstNode *right_type = _check_expression(bin->right);
            no_panic(NULL);
            if (!_types_compatible(left_type, right_type)) {
                fprintf(stderr, "Semantic error: type mismatch in binary expression '%.*s' at line %d\n",
                    (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                checker.error = true;
                checker.panic = true;
                return NULL;
            }
            return left_type;
        }

        case AST_UNARY_EXPR:
        case AST_PAREN_EXPR: {
            UnaryExprNode *un = (UnaryExprNode *)node;
            AstNode *operand_type = _check_expression(un->operand);
            no_panic(NULL);
            return operand_type;
        }

        default:
            checker.error = true;
            checker.panic = true;
            return NULL;
    }
}

b32 semantic_errors(AstNode *program) {
    //TODO check for redeclaration of global var
    // Assignment eval from the right
    // Can't assign to non-variable object
    // Figure out what to do with ! and - in unary
    _init_checker();
    _init_global();

    _check_semantics(program);

    return checker.error;
}