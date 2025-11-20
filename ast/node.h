#ifndef AST_NODE
#define AST_NODE

#include "types.h"
#include "../scanner/token.h"

typedef enum {
    // --- Top-level ---
    AST_PROGRAM,             // Root node: list of declarations

    // --- Declarations ---
    AST_FUNCTION_DECL,       // Function declaration
    AST_STRUCT_DECL,         // Struct declaration
    AST_VAR_DECL,            // Variable declaration
    AST_IMPORT,              // Import statement

    // --- Types ---
    AST_TYPE,                // General type node (with base type, array dims, etc.)
    AST_TYPE_FN,             // Function type (fn(...) -> ...)
    AST_TYPE_ARRAY,          // Array type (baseType[])
    AST_TYPE_PRIMITIVE,      // Primitive type (num, string, bool, void)
    AST_TYPE_STRUCT,         // User-defined struct type

    // --- Statements ---
    AST_BLOCK,               // Block: { ... }
    AST_EXPR_STMT,           // Expression statement
    AST_ASSIGN_STMT,         // Assignment statement
    AST_FOR_STMT,            // For loop
    AST_IF_STMT,             // If/else statement
    AST_ELIF_CLAUSE,         // elif statement
    AST_PRINT_STMT,          // Print statement
    AST_RETURN_STMT,         // Return statement
    AST_WHILE_STMT,          // While loop
    AST_BREAK_STMT,          // Break statement
    AST_CONTINUE_STMT,       // Continue statement

    // --- Expressions ---
    AST_ASSIGN_EXPR,         // Assignment expression (lvalue = expr)
    AST_BINARY_EXPR,         // Binary operation (a + b, a == b, etc.)
    AST_UNARY_EXPR,          // Unary operation (!a, -a)
    AST_CALL_EXPR,           // Function call
    AST_FIELD_ACCESS_EXPR,   // Struct field access (a.b)
    AST_INDEX_EXPR,          // Array index access (a[b])
    AST_LVALUE,              // Lvalue (identifier, field, index)
    AST_IDENTIFIER,          // Identifier (variable, function, struct name)
    AST_ARRAY_LITERAL,       // Array literal { ... }
    AST_STRUCT_LITERAL,      // Struct literal { field = value, ... }
    AST_ARGUMENT_LIST,       // List of arguments in a call
    AST_PARAMETER_LIST,      // List of parameters in a function
    AST_STRUCT_FIELD_LIST,   // List of fields in a struct literal

    // --- Literals ---
    AST_LITERAL_NUMBER,      // Number literal
    AST_LITERAL_STRING,      // String literal
    AST_LITERAL_BOOL,        // true/false
    AST_LITERAL_NULL,        // null

    // --- Utility/Other ---
    AST_PARAMETER,           // Single function parameter (type + name)
    AST_STRUCT_FIELD,        // Single struct field (type + name)
} AstNodeType;

typedef struct {
    AstNodeType ast_type;
} AstNode;

#endif