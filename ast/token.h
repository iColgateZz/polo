#ifndef TOKEN_INCLUDE
#define TOKEN_INCLUDE

#include "types.h"
#include "s8.h"

typedef enum {
    // Single-character tokens
    TOKEN_LEFT_PAREN,    // (
    TOKEN_RIGHT_PAREN,   // )
    TOKEN_LEFT_BRACE,    // {
    TOKEN_RIGHT_BRACE,   // }
    TOKEN_LEFT_BRACKET,  // [
    TOKEN_RIGHT_BRACKET, // ]
    TOKEN_COMMA,         // ,
    TOKEN_DOT,           // .
    TOKEN_SEMICOLON,     // ;
    TOKEN_EQUAL,         // =
    TOKEN_MINUS,         // -
    TOKEN_PLUS,          // +
    TOKEN_SLASH,         // /
    TOKEN_STAR,          // *
    TOKEN_BANG,          // !
    TOKEN_GREATER,       // >
    TOKEN_LESS,          // <

    // One or two character tokens
    TOKEN_BANG_EQUAL,    // !=
    TOKEN_EQUAL_EQUAL,   // ==
    TOKEN_GREATER_EQUAL, // >=
    TOKEN_LESS_EQUAL,    // <=
    TOKEN_ARROW,         // ->

    // Literals
    TOKEN_IDENTIFIER_LITERAL,
    TOKEN_NUMBER_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_TRUE_LITERAL,
    TOKEN_FALSE_LITERAL,
    TOKEN_NULL_LITERAL,

    // Keywords
    TOKEN_NUM,
    TOKEN_STRING,
    TOKEN_BOOL,
    TOKEN_VOID,
    TOKEN_FN,
    TOKEN_STRUCT,
    TOKEN_IMPORT,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_AND,
    TOKEN_OR,

    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    s8 str;
    u32 line;
} Token;

typedef struct {
    Token *items;
    usize count;
    usize capacity;
} TokenArray;

#endif