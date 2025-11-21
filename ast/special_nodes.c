#include "special_nodes.h"
#include <stdlib.h>
#include <stdio.h>

#define malloc(x) malloc(x)

AstNode *new_primitive_type_node(Token t) {
    PrimitiveTypeNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    AstNodeType type;
    switch (t.type) {
        case TOKEN_NUM:    type = AST_TYPE_NUM;    break;
        case TOKEN_STRING: type = AST_TYPE_STRING; break;
        case TOKEN_BOOL:   type = AST_TYPE_BOOL;   break;
        case TOKEN_VOID:   type = AST_TYPE_VOID;   break;
        default:           type = AST_TYPE_VOID;   break;
    }

    *n = (PrimitiveTypeNode){
        .this.ast_type = type,
        .type_token    = t
    };

    return (AstNode *)n;
}

AstNode *new_struct_type_node(Token name) {
    StructTypeNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (StructTypeNode){
        .this.ast_type = AST_TYPE_STRUCT,
        .name          = name
    };

    return (AstNode *)n;
}

AstNode *new_array_type_node(AstNode *base_type, usize dimensions) {
    ArrayTypeNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ArrayTypeNode){
        .this.ast_type = AST_TYPE_ARRAY,
        .base_type     = base_type,
        .dimensions    = dimensions
    };

    return (AstNode *)n;
}

AstNode *new_fn_type_node(AstNodeArray param_types, AstNode *return_type) {
    FnTypeNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (FnTypeNode){
        .this.ast_type = AST_TYPE_FN,
        .param_types   = param_types,
        .return_type   = return_type
    };

    return (AstNode *)n;
}

AstNode *new_parameter_list_node(AstNodeArray parameters) {
    ParameterListNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ParameterListNode){
        .this.ast_type = AST_PARAMETER_LIST,
        .parameters    = parameters
    };

    return (AstNode *)n;
}

AstNode *new_argument_list_node(AstNodeArray arguments) {
    ArgumentListNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ArgumentListNode){
        .this.ast_type = AST_ARGUMENT_LIST,
        .arguments     = arguments
    };

    return (AstNode *)n;
}

AstNode *new_struct_field_list_node(AstNodeArray fields) {
    StructFieldListNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (StructFieldListNode){
        .this.ast_type = AST_STRUCT_FIELD_LIST,
        .fields        = fields
    };

    return (AstNode *)n;
}

AstNode *new_elif_clause_node(AstNode *condition, AstNode *block) {
    ElifClauseNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ElifClauseNode){
        .this.ast_type = AST_ELIF_CLAUSE,
        .condition     = condition,
        .block         = block
    };

    return (AstNode *)n;
}

AstNode *new_if_stmt_node(AstNode *condition, AstNode *then_block, AstNodeArray elifs, AstNode *else_block) {
    IfStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (IfStmtNode){
        .this.ast_type = AST_IF_STMT,
        .condition     = condition,
        .then_block    = then_block,
        .elifs         = elifs,
        .else_block    = else_block
    };

    return (AstNode *)n;
}

AstNode *new_for_stmt_node(AstNode *init, AstNode *condition, AstNode *increment, AstNode *body) {
    ForStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ForStmtNode){
        .this.ast_type = AST_FOR_STMT,
        .init          = init,
        .condition     = condition,
        .increment     = increment,
        .body          = body
    };

    return (AstNode *)n;
}

AstNode *new_while_stmt_node(AstNode *condition, AstNode *body) {
    WhileStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (WhileStmtNode){
        .this.ast_type = AST_WHILE_STMT,
        .condition     = condition,
        .body          = body
    };

    return (AstNode *)n;
}

AstNode *new_assign_stmt_node(AstNode *lvalue, AstNode *value) {
    AssignStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (AssignStmtNode){
        .this.ast_type = AST_ASSIGN_STMT,
        .lvalue        = lvalue,
        .value         = value
    };

    return (AstNode *)n;
}

AstNode *new_expr_stmt_node(AstNode *expression) {
    ExprStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ExprStmtNode){
        .this.ast_type = AST_EXPR_STMT,
        .expression    = expression
    };

    return (AstNode *)n;
}

AstNode *new_print_stmt_node(AstNode *expression) {
    PrintStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (PrintStmtNode){
        .this.ast_type = AST_PRINT_STMT,
        .expression    = expression
    };

    return (AstNode *)n;
}

AstNode *new_return_stmt_node(AstNode *expression) {
    ReturnStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ReturnStmtNode){
        .this.ast_type = AST_RETURN_STMT,
        .expression    = expression
    };

    return (AstNode *)n;
}

AstNode *new_break_stmt_node(void) {
    BreakStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (BreakStmtNode){
        .this.ast_type = AST_BREAK_STMT
    };

    return (AstNode *)n;
}

AstNode *new_continue_stmt_node(void) {
    ContinueStmtNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ContinueStmtNode){
        .this.ast_type = AST_CONTINUE_STMT
    };

    return (AstNode *)n;
}

AstNode *new_block_node(AstNodeArray statements) {
    BlockNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (BlockNode){
        .this.ast_type = AST_BLOCK,
        .statements    = statements
    };

    return (AstNode *)n;
}

AstNode *new_program_node(AstNodeArray declarations) {
    ProgramNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ProgramNode){
        .this.ast_type = AST_PROGRAM,
        .declarations  = declarations
    };

    return (AstNode *)n;
}

AstNode *new_number_literal_node(Token value) {
    NumberLiteralNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (NumberLiteralNode){
        .this.ast_type = AST_LITERAL_NUMBER,
        .value         = value
    };

    return (AstNode *)n;
}

AstNode *new_string_literal_node(Token value) {
    StringLiteralNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (StringLiteralNode){
        .this.ast_type = AST_LITERAL_STRING,
        .value         = value
    };

    return (AstNode *)n;
}

AstNode *new_bool_literal_node(Token token) {
    BoolLiteralNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (BoolLiteralNode){
        .this.ast_type = AST_LITERAL_BOOL,
        .token         = token
    };

    return (AstNode *)n;
}

AstNode *new_null_literal_node(Token token) {
    NullLiteralNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (NullLiteralNode){
        .this.ast_type = AST_LITERAL_NULL,
        .token         = token
    };

    return (AstNode *)n;
}

AstNode *new_identifier_node(Token name) {
    IdentifierNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (IdentifierNode){
        .this.ast_type = AST_IDENTIFIER,
        .name          = name
    };

    return (AstNode *)n;
}

AstNode *new_binary_expr_node(AstNode *left, AstNode *right, Token op_token) {
    BinaryExprNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (BinaryExprNode){
        .this.ast_type = AST_BINARY_EXPR,
        .left          = left,
        .right         = right,
        .op_token      = op_token
    };

    return (AstNode *)n;
}

AstNode *new_unary_expr_node(AstNode *operand, Token op_token) {
    UnaryExprNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (UnaryExprNode){
        .this.ast_type = AST_UNARY_EXPR,
        .operand       = operand,
        .op_token      = op_token
    };

    return (AstNode *)n;
}

AstNode *new_paren_expr_node(AstNode *expression) {
    ParenExprNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ParenExprNode){
        .this.ast_type = AST_UNARY_EXPR,
        .expression    = expression
    };

    return (AstNode *)n;
}

AstNode *new_assign_expr_node(AstNode *lvalue, AstNode *value) {
    AssignExprNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (AssignExprNode){
        .this.ast_type = AST_ASSIGN_EXPR,
        .lvalue        = lvalue,
        .value         = value
    };

    return (AstNode *)n;
}

AstNode *new_function_decl_node(AstNode *return_type, Token name, AstNodeArray parameters, AstNode *body) {
    FunctionDeclNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (FunctionDeclNode){
        .this.ast_type = AST_FUNCTION_DECL,
        .return_type   = return_type,
        .name          = name,
        .parameters    = parameters,
        .body          = body
    };

    return (AstNode *)n;
}

AstNode *new_struct_decl_node(Token name, AstNodeArray fields) {
    StructDeclNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (StructDeclNode){
        .this.ast_type = AST_STRUCT_DECL,
        .name          = name,
        .fields        = fields
    };

    return (AstNode *)n;
}

AstNode *new_var_decl_node(AstNode *type, Token name, AstNode *initializer) {
    VarDeclNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (VarDeclNode){
        .this.ast_type = AST_VAR_DECL,
        .type          = type,
        .name          = name,
        .initializer   = initializer
    };

    return (AstNode *)n;
}

AstNode *new_import_node(Token path) {
    ImportNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ImportNode){
        .this.ast_type = AST_IMPORT,
        .path          = path
    };

    return (AstNode *)n;
}

AstNode *new_lvalue_node(AstNode *base, AstNodeArray accesses) {
    LValueNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (LValueNode){
        .this.ast_type = AST_LVALUE,
        .base          = base,
        .accesses      = accesses
    };

    return (AstNode *)n;
}

AstNode *new_field_access_node(AstNode *object, Token field_name) {
    FieldAccessNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (FieldAccessNode){
        .this.ast_type = AST_FIELD_ACCESS_EXPR,
        .object        = object,
        .field_name    = field_name
    };

    return (AstNode *)n;
}

AstNode *new_index_access_node(AstNode *array, AstNode *index) {
    IndexAccessNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (IndexAccessNode){
        .this.ast_type = AST_INDEX_EXPR,
        .array         = array,
        .index         = index
    };

    return (AstNode *)n;
}

AstNode *new_call_expr_node(AstNode *callee, AstNodeArray arguments) {
    CallExprNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (CallExprNode){
        .this.ast_type = AST_CALL_EXPR,
        .callee        = callee,
        .arguments     = arguments
    };

    return (AstNode *)n;
}

AstNode *new_array_literal_node(AstNodeArray elements) {
    ArrayLiteralNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ArrayLiteralNode){
        .this.ast_type = AST_ARRAY_LITERAL,
        .elements      = elements
    };

    return (AstNode *)n;
}

AstNode *new_struct_literal_node(AstNodeArray fields) {
    StructLiteralNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (StructLiteralNode){
        .this.ast_type = AST_STRUCT_LITERAL,
        .fields        = fields
    };

    return (AstNode *)n;
}

AstNode *new_struct_field_assign_node(Token field_name, AstNode *value) {
    StructFieldAssignNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (StructFieldAssignNode){
        .this.ast_type = AST_STRUCT_FIELD,
        .field_name    = field_name,
        .value         = value
    };

    return (AstNode *)n;
}

AstNode *new_parameter_node(AstNode *type, Token name) {
    ParameterNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ParameterNode){
        .this.ast_type = AST_PARAMETER,
        .type          = type,
        .name          = name
    };

    return (AstNode *)n;
}

AstNode *new_struct_field_node(AstNode *type, Token name) {
    StructFieldNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (StructFieldNode){
        .this.ast_type = AST_STRUCT_FIELD,
        .type          = type,
        .name          = name
    };

    return (AstNode *)n;
}

AstNode *new_error_node(Token error_token, byte *message) {
    ErrorNode *n = malloc(sizeof(*n));
    if (!n) {
        fprintf(stderr, "Buy more RAM, %s, %d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *n = (ErrorNode){
        .this.ast_type = AST_ERROR,
        .error_token   = error_token,
        .msg           = message
    };

    return (AstNode *)n;
}
