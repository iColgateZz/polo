#include "converter.h"
#include "../ast/special_nodes.h"
#include "../ast/token.h"
#include "da.h"
#include "string.h"
#include <stdio.h>

#define UNREACHABLE()   \
        do {            \
            fprintf(stderr, "Unreachable: %s, %d\n", __FILE__, __LINE__);   \
            exit(-1);   \
        } while (0)

static ConversionResult res;

void _init_converter(void) {
    res = (ConversionResult) {0};
}

usize _store_global(s8 str) {
    da_append(&res.globals, str);
    return res.globals.count - 1;
}

usize _store_constant(s8 str) {
    da_append(&res.constants, str);
    return res.constants.count - 1;
}

b32 _s8_eq(s8 s1, s8 s2) {
    return s1.len == s2.len &&
        memcmp(s1.s, s2.s, s1.len) == 0;
}

usize _find_global(s8 str) {
    for (usize i = 0; i < res.globals.count; ++i) {
        if (_s8_eq(res.globals.items[i], str)) return i;
    }

    UNREACHABLE();
}

void _convert(AstNode *node);

void _convert(AstNode *node) {
    if (!node) return;

    switch (node->ast_type) {
        case AST_PROGRAM: {
            ProgramNode *n = (ProgramNode *)node;
            for (usize i = 0; i < n->declarations.count; ++i) {
                _convert(n->declarations.items[i]);
            }
            da_append(&res.instructions, iHalt);
            break;
        }

        case AST_VAR_DECL: {
            VarDeclNode *var = (VarDeclNode *)node;
            usize global_idx = _store_global(var->name.str);

            if (var->initializer) {
                _convert(var->initializer);
            } else {
                // Push default value depending on type
                PrimitiveTypeNode *type = (PrimitiveTypeNode *)var->type;
                if (type->this.ast_type == AST_TYPE_NUM) {
                    da_append(&res.instructions, iPush_Num);
                    da_append(&res.instructions, _store_constant(s8("0")));
                } else if (type->this.ast_type == AST_TYPE_BOOL) {
                    da_append(&res.instructions, iPush_Bool);
                    da_append(&res.instructions, _store_constant(s8("false")));
                } else if (type->this.ast_type == AST_TYPE_STRING) {
                    da_append(&res.instructions, iPush_Str);
                    da_append(&res.instructions, _store_constant(s8("")));
                }
            }
            da_append(&res.instructions, iStore_Global);
            da_append(&res.instructions, global_idx);
            break;
        }

        case AST_LITERAL_NUMBER: {
            NumberLiteralNode *n = (NumberLiteralNode *)node;
            da_append(&res.instructions, iPush_Num);
            da_append(&res.instructions, _store_constant(n->value.str));
            break;
        }

        case AST_LITERAL_STRING: {
            StringLiteralNode *n = (StringLiteralNode *)node;
            da_append(&res.instructions, iPush_Str);
            da_append(&res.instructions, _store_constant(n->value.str));
            break;
        }

        case AST_LITERAL_BOOL: {
            BoolLiteralNode *n = (BoolLiteralNode *)node;
            da_append(&res.instructions, iPush_Bool);
            da_append(&res.instructions, _store_constant(n->token.str));
            break;
        }

        case AST_IDENTIFIER: {
            IdentifierNode *id = (IdentifierNode *)node;
            usize global_idx = _find_global(id->name.str);
            da_append(&res.instructions, iLoad_Global);
            da_append(&res.instructions, global_idx);
            break;
        }

        case AST_ASSIGN_EXPR: {
            AssignExprNode *assign = (AssignExprNode *)node;
            // Evaluate RHS first
            _convert(assign->value);

            // Then LHS (should be identifier)
            IdentifierNode *id = (IdentifierNode *)assign->lvalue;
            usize global_idx = _find_global(id->name.str);
            da_append(&res.instructions, iStore_Global);
            da_append(&res.instructions, global_idx);

            // For assignment as an expression, also load the value back
            da_append(&res.instructions, iLoad_Global);
            da_append(&res.instructions, global_idx);
            break;
        }

        case AST_BINARY_EXPR: {
            BinaryExprNode *bin = (BinaryExprNode *)node;
            _convert(bin->left);
            _convert(bin->right);

            switch (bin->op_token.type) {
                case TOKEN_PLUS:          da_append(&res.instructions, iAdd);  break;
                case TOKEN_MINUS:         da_append(&res.instructions, iSub);  break;
                case TOKEN_STAR:          da_append(&res.instructions, iMul);  break;
                case TOKEN_SLASH:         da_append(&res.instructions, iDiv);  break;
                case TOKEN_AND:           da_append(&res.instructions, iAnd);  break;
                case TOKEN_OR:            da_append(&res.instructions, iOr);   break;
                case TOKEN_EQUAL_EQUAL:   da_append(&res.instructions, iEq);   break;
                case TOKEN_BANG_EQUAL:    da_append(&res.instructions, iNeq);  break;
                case TOKEN_GREATER:       da_append(&res.instructions, iGt);   break;
                case TOKEN_GREATER_EQUAL: da_append(&res.instructions, iGte);  break;
                case TOKEN_LESS:          da_append(&res.instructions, iLt);   break;
                case TOKEN_LESS_EQUAL:    da_append(&res.instructions, iLte);  break;
                default: UNREACHABLE();
            }
            break;
        }

        case AST_UNARY_EXPR: {
            UnaryExprNode *un = (UnaryExprNode *)node;
            _convert(un->operand);

            switch (un->op_token.type) {
                case TOKEN_BANG:  da_append(&res.instructions, iNot); break;
                case TOKEN_MINUS: da_append(&res.instructions, iNeg); break;
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
    _convert(program);
    return res;
}