#ifndef PARSER_INCLUDE
#define PARSER_INCLUDE

#include "node.h"
#include "token.h"

typedef struct {
    AstNode *program;
    b32 error;
} ParseResult;

ParseResult parse(TokenArray tokens);
void free_ast(AstNode *program);

#endif