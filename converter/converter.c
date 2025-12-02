#include "converter.h"
#include "../ast/special_nodes.h"
#include "da.h"
#include <string.h>
#include <stdio.h>
#include "macros.h"

static ConversionResult res;

void _init_converter(void) {
    res = (ConversionResult) {0};
}

isize _store_global(s8 str) {
    da_append(&res.globals, str);
    return res.globals.count - 1;
}

b32 _s8_to_b32(s8 str) {
    return str.s[0] == 't';
}

Number _s8_to_num(s8 str) {
    byte buf[256];
    usize n = (usize)str.len < sizeof(buf) - 1 ? str.len : sizeof(buf) - 1;
    memcpy(buf, str.s, n);
    buf[n] = '\0';

    if (strchr(buf, '.')) {
        f64 d = strtod(buf, NULL);
        return new_num_float(d);
    } else {
        i32 i = strtol(buf, NULL, 10);
        return new_num_int(i);
    }
}

isize _store_constant(s8 str, ValueType type) {
    switch (type) {
        case VAL_STR:
            da_append(&res.constants, new_val_str(str)); 
            break;
        case VAL_NUM:
            da_append(&res.constants, new_val_num(_s8_to_num(str))); 
            break;
        case VAL_BOOL:
            da_append(&res.constants, new_val_bool(_s8_to_b32(str)));
            break;
    
        default: UNREACHABLE();
    }

    return res.constants.count - 1;
}

static inline
b32 _s8_eq(s8 s1, s8 s2) {
    return s1.len == s2.len &&
        memcmp(s1.s, s2.s, s1.len) == 0;
}

isize _find_global(s8 str) {
    for (usize i = 0; i < res.globals.count; ++i) {
        if (_s8_eq(res.globals.items[i], str)) return i;
    }

    UNREACHABLE();
}

static inline
b32 _token_eq(Token a, Token b) {
    return a.str.len == b.str.len &&
        memcmp(a.str.s, b.str.s, a.str.len) == 0;
}

typedef struct {
    Token name;
    isize scope;
} LocalSymbol;

typedef struct {
    LocalSymbol *items;
    usize count;
    usize capacity;
} LocalStack;

typedef struct {
    LocalStack locals;
    isize scope;
    b32 in_func;
    usize fn_idx;
} Info;

static Info info;

static void _init_info(void) {
    info = (Info) {0};
}

static inline
void scope(void) {
    info.scope++;
}

static inline
void rm_scope(void) {
    info.scope--;
}

static void _clear_local(void) {
    info.locals.count = 0;
}

static isize _push_local(LocalSymbol local) {
    da_append(&info.locals, local);
    return info.locals.count - 1;
}

static LocalSymbol _new_local(Token name) {
    return (LocalSymbol) {.name = name, .scope = info.scope };
}

static isize _lookup_local(Token name) {
    for (usize i = 0; i < info.locals.count; ++i) {
        if (_token_eq(info.locals.items[i].name, name) 
            && info.locals.items[i].scope <= info.scope)
            return i;
    }
    return -1;
}

static usize _lookup_function(Token name) {
    for (usize i = 0; i < res.functions.count; ++i) {
        if (_token_eq(res.functions.items[i].name, name))
            return i;
    }
    UNREACHABLE();
}

static usize _add_function(Token name, usize address) {
    FunctionSymbol s = {.name = name, .address = address};
    for (usize i = 0; i < res.functions.count; ++i) {
        if (_token_eq(res.functions.items[i].name, name)) {
            if (res.functions.items[i].instructions.count == 0) {
                res.functions.items[i] = s;
            }
            return i;
        }
    }

    da_append(&res.functions, s);
    return res.functions.count - 1;
}

static void _append_i(usize i) {
    if (info.in_func) {
        InstructionSet *set = &res.functions.items[info.fn_idx].instructions;
        da_append(set, i);
    } else {
        da_append(&res.instructions, i);
    }
}

static usize _get_label(void) {
    return res.functions.items[info.fn_idx].instructions.count;
}

void _convert(AstNode *node) {
    if (!node) return;

    switch (node->ast_type) {
        case AST_PROGRAM: {
            ProgramNode *n = (ProgramNode *)node;
            for (usize i = 0; i < n->declarations.count; ++i) {
                _convert(n->declarations.items[i]);
            }
            _append_i(iHalt);
            break;
        }

        case AST_FUNCTION_DECL: {
            FunctionDeclNode *fn = (FunctionDeclNode *)node;

            info.fn_idx = _add_function(fn->name, res.instructions.count);

            
            if (fn->body) {
                AstNodeArray params = ((ParameterListNode *)fn->parameters)->parameters;
                for (usize i = 0; i < params.count; ++i) {
                    ParameterNode *param = (ParameterNode *)params.items[i];
                    _push_local(_new_local(param->name));
                }
                info.in_func = true;
    
                _convert(fn->body);
                _append_i(iRestore);
    
                _clear_local();
                
                info.in_func = false;
            }
            break;
        }

        case AST_BLOCK: {
            BlockNode *block = (BlockNode *)node;
            scope();
            usize old_count = info.locals.count;
            for (usize i = 0; i < block->statements.count; ++i)
                _convert(block->statements.items[i]);
            info.locals.count = old_count;
            rm_scope();
            break;
        }

        case AST_RETURN_STMT: {
            ReturnStmtNode *ret = (ReturnStmtNode *)node;
            if (ret->expression) {
                _convert(ret->expression);
            }
            _append_i(iRestore);
            break;
        }

        case AST_PRINT_STMT: {
            PrintStmtNode *p = (PrintStmtNode *)node;
            _convert(p->expression);
            _append_i(iPrint);
            break;
        }

        case AST_WHILE_STMT: {
            WhileStmtNode *w = (WhileStmtNode *)node;
            usize start_label = _get_label();

            // eval condition
            _convert(w->condition);
            _append_i(iJmpZ);
            usize lbl_end_idx = _get_label();
            // append instruction to occupy space
            _append_i(iJmpZ);

            _convert(w->body);
            _append_i(iJmp);
            _append_i(start_label);

            usize end_label = _get_label();
            // fix jump to end_label
            res.functions.items[info.fn_idx].instructions.items[lbl_end_idx] = end_label;
            break;
        }

        case AST_FOR_STMT: {
            ForStmtNode *f = (ForStmtNode *)node;
            _convert(f->init);

            usize start_label = _get_label();
            // eval condition
            _convert(f->condition);
            _append_i(iJmpZ);
            usize lbl_end_idx = _get_label();
            // append instruction to occupy space
            _append_i(iJmpZ);

            _convert(f->body);
            _convert(f->increment);
            _append_i(iJmp);
            _append_i(start_label);

            usize end_label = _get_label();
            // fix jump to end_label
            res.functions.items[info.fn_idx].instructions.items[lbl_end_idx] = end_label;
            break;
        }

        case AST_IF_STMT: {
            struct {
                usize *items;
                usize count;
                usize capacity;
            } end_indexes = {0};

            IfStmtNode *i = (IfStmtNode *)node;

            _convert(i->condition);

            _append_i(iJmpZ);
            usize end_idx = _get_label();
            // append instruction to occupy space
            _append_i(iJmpZ);

            _convert(i->then_block);
            _append_i(iJmp);
            da_append(&end_indexes, _get_label());
            // append instruction to occupy space
            _append_i(iJmp);

            usize end_label = _get_label();
            // fix jump to end_label for if-then block
            res.functions.items[info.fn_idx].instructions.items[end_idx] = end_label;

            if (i->elifs) {
                AstNodeArray elifs = ((ElifClauseListNode *)i->elifs)->elifs;
                for (usize i = 0; i < elifs.count; ++i) {
                    ElifClauseNode *elif = (ElifClauseNode *)elifs.items[i];
                    _convert(elif->condition);

                    _append_i(iJmpZ);
                    end_idx = _get_label();
                    // append instruction to occupy space
                    _append_i(iJmpZ);

                    _convert(elif->block);
                    _append_i(iJmp);
                    da_append(&end_indexes, _get_label());
                    // append instruction to occupy space
                    _append_i(iJmp);

                    end_label = _get_label();
                    // fix jump to end_label for elif-then block
                    res.functions.items[info.fn_idx].instructions.items[end_idx] = end_label;
                }
            }

            if (i->else_block) {
                _convert(i->else_block);
            }

            usize absolute_end = _get_label();
            for (usize i = 0; i < end_indexes.count; ++i) {
                res.functions.items[info.fn_idx].instructions.items[end_indexes.items[i]] = absolute_end;
            }

            da_free(end_indexes);

            break;
        }

        case AST_EXPR_STMT: {
            ExprStmtNode *e = (ExprStmtNode *)node;
            _convert(e->expression);
            break;
        }

        case AST_ASSIGN_STMT: {
            AssignStmtNode *a = (AssignStmtNode *)node;
            _convert(a->value);

            IdentifierNode *id = (IdentifierNode *)a->lvalue;
            isize local_idx = _lookup_local(id->name);
            if (local_idx >= 0) {
                _append_i(iStore_Local);
                _append_i(local_idx);
            } else {
                usize global_idx = _find_global(id->name.str);
                _append_i(iStore_Global);
                _append_i(global_idx);
            }
            break;
        }

        case AST_CALL_EXPR: {
            CallExprNode *call = (CallExprNode *)node;
            IdentifierNode *callee = (IdentifierNode *)call->callee;
            AstNodeArray args = ((ArgumentListNode *)call->arguments)->arguments;

            _append_i(iSave);

            for (usize i = 0; i < args.count; ++i)
                _convert(args.items[i]);

            usize offset = _lookup_function(callee->name);

            _append_i(iCall);
            _append_i(offset);

            break;
        }

        case AST_VAR_DECL: {
            VarDeclNode *var = (VarDeclNode *)node;

            if (var->initializer) {
                _convert(var->initializer);
            } else {
                PrimitiveTypeNode *type = (PrimitiveTypeNode *)var->type;
                if (type->this.ast_type == AST_TYPE_NUM) {
                    _append_i(iPush_Const);
                    _append_i(_store_constant(s8("0"), VAL_NUM));
                } else if (type->this.ast_type == AST_TYPE_BOOL) {
                    _append_i(iPush_Const);
                    _append_i(_store_constant(s8("false"), VAL_BOOL));
                } else if (type->this.ast_type == AST_TYPE_STRING) {
                    _append_i(iPush_Const);
                    _append_i(_store_constant(s8(""), VAL_STR));
                }
            }

            if (info.in_func) {
                isize local_idx = _push_local(_new_local(var->name));
                _append_i(iStore_Local);
                _append_i(local_idx);
            } else {
                isize global_idx = _store_global(var->name.str);
                _append_i(iStore_Global);
                _append_i(global_idx);
            }

            break;
        }

        case AST_LITERAL_NUMBER: {
            NumberLiteralNode *n = (NumberLiteralNode *)node;
            _append_i(iPush_Const);
            _append_i(_store_constant(n->value.str, VAL_NUM));
            break;
        }

        case AST_LITERAL_STRING: {
            StringLiteralNode *n = (StringLiteralNode *)node;
            _append_i(iPush_Const);
            _append_i(_store_constant(n->value.str, VAL_STR));
            break;
        }

        case AST_LITERAL_BOOL: {
            BoolLiteralNode *n = (BoolLiteralNode *)node;
            _append_i(iPush_Const);
            _append_i(_store_constant(n->token.str, VAL_BOOL));
            break;
        }

        case AST_IDENTIFIER: {
            IdentifierNode *id = (IdentifierNode *)node;
            isize local_idx = _lookup_local(id->name);
            if (local_idx >= 0) {
                _append_i(iLoad_Local);
                _append_i(local_idx);
            } else {
                usize global_idx = _find_global(id->name.str);
                _append_i(iLoad_Global);
                _append_i(global_idx);
            }
            break;
        }

        case AST_ASSIGN_EXPR: {
            AssignExprNode *assign = (AssignExprNode *)node;
            _convert(assign->value);

            IdentifierNode *id = (IdentifierNode *)assign->lvalue;
            isize local_idx = _lookup_local(id->name);
            if (local_idx >= 0) {
                _append_i(iStore_Local);
                _append_i(local_idx);
                _append_i(iLoad_Local);
                _append_i(local_idx);
            } else {
                usize global_idx = _find_global(id->name.str);
                _append_i(iStore_Global);
                _append_i(global_idx);
                _append_i(iLoad_Global);
                _append_i(global_idx);
            }
            break;
        }

        case AST_BINARY_EXPR: {
            BinaryExprNode *bin = (BinaryExprNode *)node;
            _convert(bin->left);
            _convert(bin->right);

            switch (bin->op_token.type) {
                case TOKEN_PLUS:          _append_i(iAdd);  break;
                case TOKEN_MINUS:         _append_i(iSub);  break;
                case TOKEN_STAR:          _append_i(iMul);  break;
                case TOKEN_SLASH:         _append_i(iDiv);  break;
                case TOKEN_AND:           _append_i(iAnd);  break;
                case TOKEN_OR:            _append_i(iOr);   break;
                case TOKEN_EQUAL_EQUAL:   _append_i(iEq);   break;
                case TOKEN_BANG_EQUAL:    _append_i(iNeq);  break;
                case TOKEN_GREATER:       _append_i(iGt);   break;
                case TOKEN_GREATER_EQUAL: _append_i(iGte);  break;
                case TOKEN_LESS:          _append_i(iLt);   break;
                case TOKEN_LESS_EQUAL:    _append_i(iLte);  break;
                default: UNREACHABLE();
            }
            break;
        }

        case AST_UNARY_EXPR: {
            UnaryExprNode *un = (UnaryExprNode *)node;
            _convert(un->operand);

            switch (un->op_token.type) {
                case TOKEN_BANG:  _append_i(iNot); break;
                case TOKEN_MINUS: _append_i(iNeg); break;
                default: UNREACHABLE();
            }
            break;
        }

        case AST_PAREN_EXPR: {
            ParenExprNode *paren = (ParenExprNode *)node;
            _convert(paren->expression);
            break;
        }

        default: UNREACHABLE();
    }
}

ConversionResult convert(AstNode *program) {
    _init_converter();
    _init_info();
    _convert(program);
    return res;
}
