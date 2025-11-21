#ifndef SPECIAL_NODES_INCLUDE
#define SPECIAL_NODES_INCLUDE

#include "node.h"
#include "token.h"

// --- Type Nodes ---

// Primitive type (num, string, bool, void)
typedef struct {
    AstNode this;
    Token type_token; // e.g., TOKEN_NUM, TOKEN_STRING, TOKEN_BOOL, TOKEN_VOID
} PrimitiveTypeNode;

// User-defined struct type
typedef struct {
    AstNode this;
    Token name; // struct name
} StructTypeNode;

// Array type (baseType[])
typedef struct {
    AstNode this;
    AstNode *base_type; // can be PrimitiveTypeNode, StructTypeNode, or FnTypeNode
    usize dimensions;   // number of []
} ArrayTypeNode;

// Function type (fn(...) -> ...)
typedef struct {
    AstNode this;
    AstNodeArray param_types; // array of type nodes
    AstNode *return_type;     // may be NULL for no return type (void)
} FnTypeNode;

// --- Parameter List Node ---
typedef struct {
    AstNode this;
    AstNodeArray parameters; // array of ParameterNode*
} ParameterListNode;

// --- Argument List Node ---
typedef struct {
    AstNode this;
    AstNodeArray arguments; // array of AstNode* (expressions)
} ArgumentListNode;

// --- Struct Field List Node ---
typedef struct {
    AstNode this;
    AstNodeArray fields; // array of StructFieldNode*
} StructFieldListNode;

// --- Elif Clause Node ---
typedef struct {
    AstNode this;
    AstNode *condition;
    AstNode *block; // BlockNode*
} ElifClauseNode;

// --- If Statement Node (with elifs and else) ---
typedef struct {
    AstNode this;
    AstNode *condition;
    AstNode *then_block; // BlockNode*
    AstNodeArray elifs;  // array of ElifClauseNode*
    AstNode *else_block; // BlockNode* or NULL
} IfStmtNode;

// --- For Statement Node ---
typedef struct {
    AstNode this;
    AstNode *init;      // VarDeclNode*, ExprStmtNode*, or NULL
    AstNode *condition; // Expression or NULL
    AstNode *increment; // Expression or NULL
    AstNode *body;      // BlockNode*
} ForStmtNode;

// --- While Statement Node ---
typedef struct {
    AstNode this;
    AstNode *condition;
    AstNode *body; // BlockNode*
} WhileStmtNode;

// --- Assignment Statement Node ---
typedef struct {
    AstNode this;
    AstNode *lvalue;
    AstNode *value;
} AssignStmtNode;

// --- Expression Statement Node ---
typedef struct {
    AstNode this;
    AstNode *expression;
} ExprStmtNode;

// --- Print Statement Node ---
typedef struct {
    AstNode this;
    AstNode *expression;
} PrintStmtNode;

// --- Return Statement Node ---
typedef struct {
    AstNode this;
    AstNode *expression; // may be NULL
} ReturnStmtNode;

// --- Break Statement Node ---
typedef struct {
    AstNode this;
} BreakStmtNode;

// --- Continue Statement Node ---
typedef struct {
    AstNode this;
} ContinueStmtNode;

// --- Block Node (already present, but for completeness) ---
typedef struct {
    AstNode this;
    AstNodeArray statements; // Dynamic array of AstNode*
} BlockNode;

// --- Program Node (root) ---
typedef struct {
    AstNode this;
    AstNodeArray declarations; // array of FunctionDeclNode*, StructDeclNode*, VarDeclNode*, ImportNode*
} ProgramNode;

typedef struct {
    AstNode this;
    Token value; // number token
} NumberLiteralNode;

typedef struct {
    AstNode this;
    Token value; // string token
} StringLiteralNode;

typedef struct {
    AstNode this;
    Token token;
} BoolLiteralNode;

typedef struct {
    AstNode this;
    Token token; // 'null' token
} NullLiteralNode;

typedef struct {
    AstNode this;
    Token name; // identifier token
} IdentifierNode;

typedef struct {
    AstNode this;
    AstNode *left;
    AstNode *right;
    Token op_token; // e.g., +, -, *, /, ==, !=, etc.
} BinaryExprNode;

typedef struct {
    AstNode this;
    AstNode *operand;
    Token op_token; // e.g., !, -
} UnaryExprNode;

typedef struct {
    AstNode this;
    AstNode *expression;
} ParenExprNode;

typedef struct {
    AstNode this;
    AstNode *lvalue;
    AstNode *value;
} AssignExprNode;

typedef struct {
    AstNode this;
    AstNode *return_type;
    Token name;
    AstNodeArray parameters; // ParameterNode*
    AstNode *body; // BlockNode*
} FunctionDeclNode;

typedef struct {
    AstNode this;
    Token name;
    AstNodeArray fields; // StructFieldNode*
} StructDeclNode;

typedef struct {
    AstNode this;
    AstNode *type;
    Token name;
    AstNode *initializer; // may be NULL
} VarDeclNode;

typedef struct {
    AstNode this;
    Token path; // string token
} ImportNode;

typedef struct {
    AstNode this;
    AstNode *base; // IdentifierNode* or another LValueNode*
    AstNodeArray accesses; // field/index accesses (FieldAccessNode/IndexAccessNode)
} LValueNode;

typedef struct {
    AstNode this;
    AstNode *object;
    Token field_name;
} FieldAccessNode;

typedef struct {
    AstNode this;
    AstNode *array;
    AstNode *index;
} IndexAccessNode;

typedef struct {
    AstNode this;
    AstNode *callee;
    AstNodeArray arguments; // ArgumentListNode*
} CallExprNode;

typedef struct {
    AstNode this;
    AstNodeArray elements; // expressions
} ArrayLiteralNode;

typedef struct {
    AstNode this;
    AstNodeArray fields; // StructFieldAssignNode*
} StructLiteralNode;

typedef struct {
    AstNode this;
    Token field_name;
    AstNode *value;
} StructFieldAssignNode;

typedef struct {
    AstNode this;
    AstNode *type;
    Token name;
} ParameterNode;

typedef struct {
    AstNode this;
    AstNode *type;
    Token name;
} StructFieldNode;

typedef struct {
    AstNode this;
    Token error_token;
    byte *msg;
} ErrorNode;

AstNode *new_primitive_type_node(Token t);
AstNode *new_struct_type_node(Token name);
AstNode *new_array_type_node(AstNode *base_type, usize dimensions);
AstNode *new_fn_type_node(AstNodeArray param_types, AstNode *return_type);
AstNode *new_parameter_list_node(AstNodeArray parameters);
AstNode *new_argument_list_node(AstNodeArray arguments);
AstNode *new_struct_field_list_node(AstNodeArray fields);
AstNode *new_elif_clause_node(AstNode *condition, AstNode *block);
AstNode *new_if_stmt_node(AstNode *condition, AstNode *then_block, AstNodeArray elifs, AstNode *else_block);
AstNode *new_for_stmt_node(AstNode *init, AstNode *condition, AstNode *increment, AstNode *body);
AstNode *new_while_stmt_node(AstNode *condition, AstNode *body);
AstNode *new_assign_stmt_node(AstNode *lvalue, AstNode *value);
AstNode *new_expr_stmt_node(AstNode *expression);
AstNode *new_print_stmt_node(AstNode *expression);
AstNode *new_return_stmt_node(AstNode *expression);
AstNode *new_break_stmt_node(void);
AstNode *new_continue_stmt_node(void);
AstNode *new_block_node(AstNodeArray statements);
AstNode *new_program_node(AstNodeArray declarations);
AstNode *new_number_literal_node(Token value);
AstNode *new_string_literal_node(Token value);
AstNode *new_bool_literal_node(Token token);
AstNode *new_null_literal_node(Token token);
AstNode *new_identifier_node(Token name);
AstNode *new_binary_expr_node(AstNode *left, AstNode *right, Token op_token);
AstNode *new_unary_expr_node(AstNode *operand, Token op_token);
AstNode *new_paren_expr_node(AstNode *expression);
AstNode *new_assign_expr_node(AstNode *lvalue, AstNode *value);
AstNode *new_function_decl_node(AstNode *return_type, Token name, AstNodeArray parameters, AstNode *body);
AstNode *new_struct_decl_node(Token name, AstNodeArray fields);
AstNode *new_var_decl_node(AstNode *type, Token name, AstNode *initializer);
AstNode *new_import_node(Token path);
AstNode *new_lvalue_node(AstNode *base, AstNodeArray accesses);
AstNode *new_field_access_node(AstNode *object, Token field_name);
AstNode *new_index_access_node(AstNode *array, AstNode *index);
AstNode *new_call_expr_node(AstNode *callee, AstNodeArray arguments);
AstNode *new_array_literal_node(AstNodeArray elements);
AstNode *new_struct_literal_node(AstNodeArray fields);
AstNode *new_struct_field_assign_node(Token field_name, AstNode *value);
AstNode *new_parameter_node(AstNode *type, Token name);
AstNode *new_struct_field_node(AstNode *type, Token name);
AstNode *new_error_node(Token error_token, byte *message);

#endif