#include "special_nodes.h"
#include "ast_printer.h"
#include <stdio.h>

static inline
void _indent(i32 indent) {
    for (i32 i = 0; i < indent; ++i) putchar(' ');
}

void print_ast(AstNode *node, i32 indent) {
    if (!node) {
        _indent(indent);
        printf("(null)\n");
        return;
    }

    switch (node->ast_type) {
        case AST_PROGRAM: {
            ProgramNode *prog = (ProgramNode *)node;
            _indent(indent); printf("Program\n");
            for (usize i = 0; i < prog->declarations.count; ++i) {
                print_ast(prog->declarations.items[i], indent + 2);
            }
            break;
        }
        case AST_VAR_DECL: {
            VarDeclNode *var = (VarDeclNode *)node;
            _indent(indent); printf("VarDecl: %.*s\n", (i32)var->name.str.len, var->name.str.s);
            print_ast(var->type, indent + 2);
            if (var->initializer) {
                _indent(indent + 2); printf("Initializer:\n");
                print_ast(var->initializer, indent + 4);
            }
            break;
        }
        case AST_TYPE_NUM:
        case AST_TYPE_STRING:
        case AST_TYPE_BOOL:
        case AST_TYPE_VOID: {
            PrimitiveTypeNode *type = (PrimitiveTypeNode *)node;
            _indent(indent); printf("Type: %.*s\n", (i32)type->type_token.str.len, type->type_token.str.s);
            break;
        }
        case AST_TYPE_STRUCT: {
            StructTypeNode *type = (StructTypeNode *)node;
            _indent(indent); printf("Type: %.*s\n", (i32)type->name.str.len, type->name.str.s);
            break;
        }
        case AST_LITERAL_NUMBER: {
            NumberLiteralNode *num = (NumberLiteralNode *)node;
            _indent(indent); printf("Number: %.*s\n", (i32)num->value.str.len, num->value.str.s);
            break;
        }
        case AST_LITERAL_STRING: {
            StringLiteralNode *str = (StringLiteralNode *)node;
            _indent(indent); printf("String: %.*s\n", (i32)str->value.str.len, str->value.str.s);
            break;
        }
        case AST_IDENTIFIER: {
            IdentifierNode *id = (IdentifierNode *)node;
            _indent(indent); printf("Identifier: %.*s\n", (i32)id->name.str.len, id->name.str.s);
            break;
        }
        case AST_ERROR: {
            ErrorNode *err = (ErrorNode *)node;
            _indent(indent); printf("Error: %s at line %d\n", err->msg, err->error_token.line);
            break;
        }
        default:
            _indent(indent); printf("Unknown node type %d\n", node->ast_type);
            break;
    }
}