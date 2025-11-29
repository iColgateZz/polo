#include "ast_checker.h"
#include "special_nodes.h"
#include "token.h"
#include <stdio.h>
#include "da.h"
#include <stdarg.h>
#include "macros.h"

typedef struct {
    b32 error;
    b32 panic;
} Checker;

static Checker checker;

static inline
void _init_checker(void) {
    checker = (Checker) {0};
}

#define no_panic(retval) do { if (checker.panic) return retval; } while (0)

static 
void _semantic_error(const byte *fmt, ...) {
    fprintf(stderr, "Semantic error: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    checker.error = true;
    checker.panic = true;
}

static
b32 _any_type(AstNode *node, i32 count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; ++i) {
        AstNodeType type = va_arg(args, AstNodeType);
        if (node->ast_type == type) {
            va_end(args);
            return true;
        }
    }

    va_end(args);
    return false;
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

static
AstNode *_lookup_global(Token name) {
    for (size_t i = 0; i < global_symbols.count; ++i) {
        if (global_symbols.items[i].name.str.len == name.str.len &&
            memcmp(global_symbols.items[i].name.str.s, name.str.s, name.str.len) == 0) {
            return global_symbols.items[i].type;
        }
    }
    return NULL;
}

static inline
void _add_global(Token name, AstNode *type) {
    Symbol s = {.name = name, .type = type};
    da_append(&global_symbols, s);
}

typedef struct {
    Token name;
    AstNode *decl;
    b32 proto;
} FunctionSymbol;

typedef struct {
    FunctionSymbol *items;
    size_t count;
    size_t capacity;
} FunctionTable;

static FunctionTable global_functions;

static void _init_functions(void) {
    global_functions = (FunctionTable){0};
}

static FunctionSymbol *_lookup_function(Token name) {
    for (size_t i = 0; i < global_functions.count; ++i) {
        if (global_functions.items[i].name.str.len == name.str.len &&
            memcmp(global_functions.items[i].name.str.s, name.str.s, name.str.len) == 0) {
            return &global_functions.items[i];
        }
    }
    return NULL;
}

static void _add_function(Token name, AstNode *decl, b32 proto) {
    FunctionSymbol s = {.name = name, .decl = decl, .proto = proto};
    da_append(&global_functions, s);
}

static PrimitiveTypeNode sentinel_type;
static inline
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

static inline
b32 _types_compatible(AstNode *lhs_type, AstNode *rhs_type) {
    if (!lhs_type || !rhs_type) return false;
    return _get_type_of(lhs_type)->ast_type == _get_type_of(rhs_type)->ast_type;
}

static
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

        case AST_FUNCTION_DECL: {
            FunctionDeclNode *fn = (FunctionDeclNode *)node;

            FunctionSymbol *fn_symbol = _lookup_function(fn->name);
            if (fn_symbol) {
                if (!fn_symbol->proto) {
                    _semantic_error("redeclaration of function '%.*s' at line %d",
                        (i32)fn->name.str.len, fn->name.str.s, fn->name.line);
                    return NULL;
                }
            }

            _add_function(fn->name, node, fn->body == NULL);

            // Check parameter names for duplicates
            // for (usize i = 0; i < fn->parameters.count; ++i) {
            //     ParameterNode *param_i = (ParameterNode *)fn->parameters.items[i];
            //     for (usize j = i + 1; j < fn->parameters.count; ++j) {
            //         ParameterNode *param_j = (ParameterNode *)fn->parameters.items[j];
            //         if (param_i->name.str.len == param_j->name.str.len &&
            //             memcmp(param_i->name.str.s, param_j->name.str.s, param_i->name.str.len) == 0) {
            //             _semantic_error("duplicate parameter name '%.*s' in function '%.*s' at line %d",
            //                 (i32)param_i->name.str.len, param_i->name.str.s,
            //                 (i32)fn->name.str.len, fn->name.str.s, fn->name.line);
            //             return NULL;
            //         }
            //     }
            // }

            if (fn->body) {
                _check_node(fn->body);
                no_panic(NULL);
            }

            return NULL;
        }

        case AST_BLOCK: {
            BlockNode *block = (BlockNode *)node;
            for (usize i = 0; i < block->statements.count; ++i) {
                _check_node(block->statements.items[i]);
                no_panic(NULL);
            }
            return NULL;
        }

        case AST_PARAMETER: {
            return node;
        }

        case AST_CALL_EXPR: {
            CallExprNode *call = (CallExprNode *)node;
            IdentifierNode *callee = (IdentifierNode *)call->callee;
            // Lookup function
            FunctionSymbol *fn = _lookup_function(callee->name);
            if (!fn) {
                _semantic_error("call to undefined function '%.*s' at line %d", 
                    (i32)callee->name.str.len, callee->name.str.s, callee->name.line);
                return NULL;
            }
            // Check argument count and types
            // ...
            FunctionDeclNode *fn_decl = (FunctionDeclNode *)fn->decl;
            return fn_decl->return_type;
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
            AstNode *rhs_type = _check_node(assign->value);
            no_panic(NULL);
            AstNode *lhs_type = _check_node(assign->lvalue);
            no_panic(NULL);

            if (lhs_type->ast_type != AST_IDENTIFIER) {
                _semantic_error("cannot assign to an expression");
                return NULL;
            }

            if (!_types_compatible(lhs_type, rhs_type)) {
                _semantic_error("type mismatch in assignment at line %d",
                    ((IdentifierNode *)assign->lvalue)->name.line);
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

            // enforce types depending on the operation
            // here both values have the same type
            switch (bin->op_token.type) {
                case TOKEN_PLUS:
                case TOKEN_MINUS:
                case TOKEN_STAR:
                case TOKEN_SLASH:
                    // operations only defined for numbers
                    // they produce a number
                    if (!_any_type(left_type, 2, AST_TYPE_NUM, AST_LITERAL_NUMBER)) {
                        _semantic_error("Operation '%.*s' is only defined for numbers. Error at line %d",
                            (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                        return NULL;
                    }
                    break;

                case TOKEN_AND:
                case TOKEN_OR:
                case TOKEN_EQUAL_EQUAL:
                case TOKEN_BANG_EQUAL:
                case TOKEN_GREATER:
                case TOKEN_GREATER_EQUAL:
                case TOKEN_LESS:
                case TOKEN_LESS_EQUAL:
                    // operations defined for numbers and booleans
                    // operations produce a boolean value
                    if (!_any_type(left_type, 4, AST_TYPE_NUM, AST_LITERAL_NUMBER, AST_TYPE_BOOL, AST_LITERAL_BOOL)) {
                        _semantic_error("Operation '%.*s' is only defined for numbers and booleans. Error at line %d",
                            (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                        return NULL;
                    }
                    sentinel_type.this.ast_type = AST_TYPE_BOOL;
                    return (AstNode *)&sentinel_type;

                default: UNREACHABLE();
            }

            return left_type;
        }

        case AST_UNARY_EXPR: {
            UnaryExprNode *un = (UnaryExprNode *)node;
            AstNode *operand_type = _check_node(un->operand);
            no_panic(NULL);

            if (!_any_type(operand_type, 2, AST_TYPE_BOOL, AST_LITERAL_BOOL)
                && un->op_token.type == TOKEN_BANG) {
                _semantic_error("type mismatch in unary expression '%.*s' at line %d",
                    (i32)un->op_token.str.len, un->op_token.str.s, un->op_token.line);
                return NULL;
            }

            if (!_any_type(operand_type, 2, AST_TYPE_NUM, AST_LITERAL_NUMBER)
                && un->op_token.type == TOKEN_MINUS) {
                _semantic_error("type mismatch in unary expression '%.*s' at line %d",
                    (i32)un->op_token.str.len, un->op_token.str.s, un->op_token.line);
                return NULL;
            }

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
    _init_checker();
    _init_global();
    _init_functions();

    _check_node(program);

    return checker.error;
}