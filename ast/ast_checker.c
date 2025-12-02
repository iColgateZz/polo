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
    b32 in_func;
    AstNode *fn_ret_type;
    b32 had_return;
    isize scope;
} Checker;

static Checker checker;

static inline
void scope(void) {
    checker.scope++;
}

static inline
void rm_scope(void) {
    checker.scope--;
}

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

static inline
b32 _token_eq(Token a, Token b) {
    return a.str.len == b.str.len &&
        memcmp(a.str.s, b.str.s, a.str.len) == 0;
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
        if (_token_eq(global_symbols.items[i].name, name))
            return global_symbols.items[i].type;
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
    AstNode *type;
    isize scope;
} LocalSymbol;

typedef struct {
    LocalSymbol *items;
    usize count;
    usize capacity;
} LocalStack;

static LocalStack local_symbols;

static void _init_local(void) {
    local_symbols = (LocalStack) {0};
}

static void _clear_local(void) {
    local_symbols.count = 0;
}

static void _push_local(LocalSymbol local) {
    da_append(&local_symbols, local);
}

static LocalSymbol _new_local(Token name, AstNode *type) {
    return (LocalSymbol) {
        .name = name,
        .type = type,
        .scope = checker.scope,
    };
}

static AstNode *_lookup_local(Token name) {
    for (size_t i = 0; i < local_symbols.count; ++i) {
        if (_token_eq(local_symbols.items[i].name, name) 
            && local_symbols.items[i].scope <= checker.scope)
            return local_symbols.items[i].type;
    }
    return NULL;
}

typedef struct {
    Token name;
    AstNode *decl;
    b32 proto;
    usize idx;
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
        if (_token_eq(global_functions.items[i].name, name))
            return &global_functions.items[i];
    }
    return NULL;
}

static void _add_function(Token name, AstNode *decl, b32 proto, isize idx) {
    if (idx >= 0) {
        global_functions.items[idx].proto = proto;
        return;
    }
    idx = global_functions.count;
    FunctionSymbol s = {.name = name, .decl = decl, .proto = proto, .idx = idx};
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
            return node;
    }
}

static inline
b32 _types_compatible(AstNode *lhs_type, AstNode *rhs_type) {
    if (!lhs_type || !rhs_type) return false;
    return _get_type_of(lhs_type)->ast_type == _get_type_of(rhs_type)->ast_type;
}

static AstNode *_lookup_var(Token name) {
    AstNode *local = _lookup_local(name);
    if (local) return local;

    return _lookup_global(name);
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

            AstNodeArray params = ((ParameterListNode *) fn->parameters)->parameters;
            for (usize i = 0; i < params.count; ++i) {
                ParameterNode *param_i = (ParameterNode *)params.items[i];
                for (usize j = i + 1; j < params.count; ++j) {
                    ParameterNode *param_j = (ParameterNode *)params.items[j];
                    if (_token_eq(param_i->name, param_j->name)) {
                        _semantic_error("duplicate parameter name '%.*s' in function '%.*s' at line %d",
                            (i32)param_i->name.str.len, param_i->name.str.s,
                            (i32)fn->name.str.len, fn->name.str.s, fn->name.line);
                        return NULL;
                    }
                }
            }

            FunctionSymbol *fn_symbol = _lookup_function(fn->name);
            if (fn_symbol) {
                if (!fn_symbol->proto) {
                    _semantic_error("redeclaration of function '%.*s' at line %d",
                        (i32)fn->name.str.len, fn->name.str.s, fn->name.line);
                    return NULL;
                }
            }

            if (fn_symbol) {
                FunctionDeclNode * fn_sym_decl = (FunctionDeclNode *)fn_symbol->decl;
                AstNode *fn_sym_type = fn_sym_decl->return_type;
                if (!_types_compatible(fn_sym_type, fn->return_type)) {
                    _semantic_error("return type of function '%.*s' at line %d does "
                                    "not match the one defined previously at line %d",
                        (i32)fn->name.str.len, fn->name.str.s, fn->name.line,
                        ((PrimitiveTypeNode *)fn_sym_type)->type_token.line);
                    return NULL;
                }
            }

            if (fn_symbol) {
                FunctionDeclNode * fn_sym_decl = (FunctionDeclNode *)fn_symbol->decl;
                AstNodeArray fn_sym_params = ((ParameterListNode *)fn_sym_decl->parameters)->parameters;

                if (fn_sym_params.count != params.count) {
                    _semantic_error("number of parameters of function '%.*s' at line %d "
                                    "does not match the one defined previously at line %d",
                        (i32)fn->name.str.len, fn->name.str.s, fn->name.line,
                            fn_sym_decl->name.line);
                    return NULL;
                }

                for (usize i = 0; i < params.count; ++i) {
                    ParameterNode *param_a = (ParameterNode *)params.items[i];
                    ParameterNode *param_b = (ParameterNode *)fn_sym_params.items[i];

                    if (!_token_eq(param_a->name, param_b->name)) {
                        _semantic_error("name of parameter '%.*s' of function '%.*s' at line %d "
                                        "does not match the name of parameter '%.*s' at line %d",
                            (i32)param_a->name.str.len, param_a->name.str.s,
                            (i32)fn->name.str.len, fn->name.str.s, fn->name.line,
                            (i32)param_b->name.str.len, param_b->name.str.s,
                             fn_sym_decl->name.line);
                        return NULL;
                    }

                    if (!_types_compatible(param_a->type, param_b->type)) {
                        _semantic_error("type of parameter '%.*s' of function '%.*s' at line %d "
                                        "does not match the type of parameter '%.*s' at line %d",
                            (i32)param_a->name.str.len, param_a->name.str.s,
                            (i32)fn->name.str.len, fn->name.str.s, fn->name.line,
                            (i32)param_b->name.str.len, param_b->name.str.s,
                             fn_sym_decl->name.line);
                        return NULL;
                    }
                }
            }

            isize idx = fn_symbol ? fn_symbol->idx : -1;
            _add_function(fn->name, node, fn->body == NULL, idx);

            if (fn->body) {
                for (usize i = 0; i < params.count; ++i) {
                    ParameterNode *param = (ParameterNode *)params.items[i];
                    _push_local(_new_local(param->name, param->type));
                }
                checker.in_func = true;
                checker.fn_ret_type = fn->return_type;
                checker.had_return = false;

                _check_node(fn->body);

                if (fn->return_type->ast_type != AST_TYPE_VOID &&
                    !checker.had_return) {
                    _semantic_error("function '%.*s' at line %d does "
                                    "not have a return statement",
                        (i32)fn->name.str.len, fn->name.str.s, fn->name.line);
                    return NULL;
                }

                checker.in_func = false;
                _clear_local();
                no_panic(NULL);
            }

            return NULL;
        }

        case AST_BLOCK: {
            BlockNode *block = (BlockNode *)node;
            scope();
            for (usize i = 0; i < block->statements.count; ++i) {
                _check_node(block->statements.items[i]);
                checker.panic = false;
            }
            rm_scope();
            return NULL;
        }

        case AST_RETURN_STMT: {
            ReturnStmtNode *ret = (ReturnStmtNode *)node;
            checker.had_return = true;

            if (ret->expression && checker.fn_ret_type->ast_type == AST_TYPE_VOID ) {
                _semantic_error("returning from a void function");
                return NULL;
            }

            if (!ret->expression && checker.fn_ret_type->ast_type != AST_TYPE_VOID ) {
                _semantic_error("not returning from a non-void function");
                return NULL;
            }

            if (!ret->expression) return NULL;

            AstNode *ret_type = _check_node(ret->expression);
            no_panic(ret_type);

            if (!_types_compatible(ret_type, checker.fn_ret_type)) {
                _semantic_error("the type of returned value does not match "
                                "the return type of function");
                return NULL;
            }

            return NULL;
        }

        case AST_PRINT_STMT: {
            PrintStmtNode *p = (PrintStmtNode *)node;
            AstNode *t = _check_node(p->expression);

            if (t->ast_type == AST_TYPE_VOID) {
                _semantic_error("cannot print argument of void type");
                return NULL;
            }
            return NULL;
        }

        case AST_WHILE_STMT: {
            WhileStmtNode *w = (WhileStmtNode *)node;
            
            AstNode *cond_type = _check_node(w->condition);
            if (cond_type->ast_type != AST_TYPE_BOOL) {
                _semantic_error("condition in while loop must evaluate to a boolean value");
                return NULL;
            }

            return _check_node(w->body);
        }

        case AST_FOR_STMT: {
            ForStmtNode *f = (ForStmtNode *)node;
            scope();
            AstNode *init = _check_node(f->init);
            no_panic(init);

            AstNode *cond_type = _check_node(f->condition);
            no_panic(cond_type);
            if (cond_type) {
                if (cond_type->ast_type != AST_TYPE_BOOL) {
                    _semantic_error("condition in for loop must evaluate to a boolean value");
                    return NULL;
                }
            }

            AstNode *increment = _check_node(f->increment);
            no_panic(increment);
            rm_scope();
            return _check_node(f->body);
        }

        case AST_EXPR_STMT: {
            ExprStmtNode *e = (ExprStmtNode *)node;
            return _check_node(e->expression);
        }

        case AST_ASSIGN_STMT: {
            AssignStmtNode *a = (AssignStmtNode *)node;
            AstNode *lval_type = _check_node(a->lvalue);
            no_panic(lval_type);
            AstNode *rval_type = _check_node(a->value);
            no_panic(rval_type);

            if (!_types_compatible(lval_type, rval_type)) {
                _semantic_error("type mismatch in assignment stmt at line %d", 
                    ((IdentifierNode *)a->lvalue)->name.line);
                return NULL;
            }

            return NULL;
        }

        case AST_IF_STMT: {
            IfStmtNode *i = (IfStmtNode *)node;

            AstNode *if_cond = _check_node(i->condition);
            no_panic(if_cond);
            if (if_cond->ast_type != AST_TYPE_BOOL) {
                _semantic_error("condition in if stmt must evaluate to a boolean value");
                return NULL;
            }

            _check_node(i->then_block);

            if (i->elifs) {
                AstNodeArray elifs = ((ElifClauseListNode *)i->elifs)->elifs;
                for (usize i = 0; i < elifs.count; ++i) {
                    ElifClauseNode *elif = (ElifClauseNode *)elifs.items[i];
                    AstNode *elif_cond = _check_node(elif->condition);
                    no_panic(elif_cond);
                    if (elif_cond->ast_type != AST_TYPE_BOOL) {
                        _semantic_error("condition in elif stmt must evaluate to a boolean value");
                        return NULL;
                    }

                    _check_node(elif->block);
                }
            }

            if (i->else_block) {
                _check_node(i->else_block);
            }

            return NULL;
        }

        case AST_CALL_EXPR: {
            CallExprNode *call = (CallExprNode *)node;
            IdentifierNode *callee = (IdentifierNode *)call->callee;
            AstNodeArray args = ((ArgumentListNode *)call->arguments)->arguments;

            FunctionSymbol *fn = _lookup_function(callee->name);
            if (!fn) {
                _semantic_error("call to undefined function '%.*s' at line %d", 
                    (i32)callee->name.str.len, callee->name.str.s, callee->name.line);
                return NULL;
            }

            FunctionDeclNode *fn_decl = (FunctionDeclNode *)fn->decl;
            AstNodeArray params = ((ParameterListNode *)fn_decl->parameters)->parameters;
            if (params.count != args.count) {
                _semantic_error("Number of arguments to '%.*s' at line %d "
                                "does not match the number of parameters "
                                "of '%.*s' at line %d", 
                    (i32)callee->name.str.len, callee->name.str.s, callee->name.line,
                    (i32)fn_decl->name.str.len, fn_decl->name.str.s, fn_decl->name.line);
                return NULL;
            }

            for (usize i = 0; i < args.count; ++i) {
                ParameterNode *param = (ParameterNode *)params.items[i];
                AstNode *arg_type = _check_node(args.items[i]);
                no_panic(arg_type);

                if (!_types_compatible(param->type, arg_type)) {
                    _semantic_error("The type of argument(idx: %d) for function '%.*s' at line %d "
                                    "does not match the type of parameter '%.*s' "
                                    "of function '%.*s' at line %d", 
                        i, (i32)callee->name.str.len, callee->name.str.s, callee->name.line,
                        (i32)param->name.str.len, param->name.str.s, 
                        (i32)fn_decl->name.str.len, fn_decl->name.str.s, fn_decl->name.line);
                    return NULL;
                }
            }

            return fn_decl->return_type;
        }

        case AST_VAR_DECL: {
            VarDeclNode *var = (VarDeclNode *)node;
            
            if (var->type->ast_type == AST_TYPE_VOID) {
                _semantic_error("variable '%.*s' of type 'void' at line %d",
                    (i32)var->name.str.len, var->name.str.s, var->name.line);
                return NULL;
            }

            if (!checker.in_func) {
                if (_lookup_global(var->name)) {
                    _semantic_error("redeclaration of global variable '%.*s' at line %d",
                        (i32)var->name.str.len, var->name.str.s, var->name.line);
                    return NULL;
                }
    
                _add_global(var->name, var->type);
            } else {
                if (_lookup_local(var->name)) {
                    _semantic_error("redeclaration of local variable '%.*s' at line %d",
                        (i32)var->name.str.len, var->name.str.s, var->name.line);
                    return NULL;
                }

                _push_local(_new_local(var->name, var->type));
            }

            if (var->initializer) {
                AstNode *init_type = _check_node(var->initializer);
                no_panic(init_type);
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
            return node;

        case AST_IDENTIFIER: {
            IdentifierNode *id = (IdentifierNode *)node;
            AstNode *type = _lookup_var(id->name);
            if (!type) {
                _semantic_error("use of unknown variable '%.*s' at line %d",
                    (i32)id->name.str.len, id->name.str.s, id->name.line);
                return NULL;
            }

            return type;
        }

        case AST_ASSIGN_EXPR: {
            AssignExprNode *assign = (AssignExprNode *)node;
            AstNode *rhs_type = _check_node(assign->value);
            no_panic(rhs_type);
            AstNode *lhs_type = _check_node(assign->lvalue);
            no_panic(lhs_type);

            if (assign->lvalue->ast_type != AST_IDENTIFIER) {
                _semantic_error("cannot assign to an expression. "
                                "The type was %d", lhs_type->ast_type);
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
            no_panic(left_type);
            AstNode *right_type = _check_node(bin->right);
            no_panic(right_type);

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
                    // num, num -> num
                    if (!_any_type(left_type, 2, AST_TYPE_NUM, AST_LITERAL_NUMBER)) {
                        _semantic_error("Operation '%.*s' is only defined for numbers. Error at line %d",
                            (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                        return NULL;
                    }
                    break;

                case TOKEN_GREATER:
                case TOKEN_GREATER_EQUAL:
                case TOKEN_LESS:
                case TOKEN_LESS_EQUAL:
                    // num, num -> bool
                    if (!_any_type(left_type, 2, AST_TYPE_NUM, AST_LITERAL_NUMBER)) {
                        _semantic_error("Operation '%.*s' is only defined for numbers. Error at line %d",
                            (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                        return NULL;
                    }
                    // enforce bool type
                    sentinel_type.this.ast_type = AST_TYPE_BOOL;
                    return (AstNode *)&sentinel_type;

                case TOKEN_AND:
                case TOKEN_OR:
                    // bool, bool -> bool
                    if (!_any_type(left_type, 2, AST_TYPE_BOOL, AST_LITERAL_BOOL)) {
                        _semantic_error("Operation '%.*s' is only defined for booleans. Error at line %d",
                            (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                        return NULL;
                    }
                    break;

                case TOKEN_EQUAL_EQUAL:
                case TOKEN_BANG_EQUAL:
                    // num, num -> bool or bool, bool -> bool
                    if (!_any_type(left_type, 4, AST_TYPE_NUM, AST_LITERAL_NUMBER, AST_TYPE_BOOL, AST_LITERAL_BOOL)) {
                        _semantic_error("Operation '%.*s' is only defined for numbers and booleans. Error at line %d",
                            (i32)bin->op_token.str.len, bin->op_token.str.s, bin->op_token.line);
                        return NULL;
                    }
                    // enforce bool type
                    sentinel_type.this.ast_type = AST_TYPE_BOOL;
                    return (AstNode *)&sentinel_type;

                default: UNREACHABLE();
            }

            return left_type;
        }

        case AST_UNARY_EXPR: {
            UnaryExprNode *un = (UnaryExprNode *)node;
            AstNode *operand_type = _check_node(un->operand);
            no_panic(operand_type);

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
            no_panic(expr_type);
            return expr_type;
        }

        default:
            _semantic_error("unknown node type");
            return NULL;
    }
}

void _free_checker(void) {
    if (global_symbols.count > 0) {
        da_free(global_symbols);
    }
    if (global_functions.count > 0) {
        da_free(global_functions);
    }
    if (local_symbols.count > 0) {
        da_free(local_symbols);
    }
}

b32 semantic_errors(AstNode *program) {
    _init_checker();
    _init_global();
    _init_functions();
    _init_local();

    _check_node(program);

    _free_checker();

    return checker.error;
}
