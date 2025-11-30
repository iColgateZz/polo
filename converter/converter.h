#ifndef CONVERTER_INCLUDE
#define CONVERTER_INCLUDE

#include "instructions.h"
#include "../ast/node.h"
#include "../ast/token.h"
#include "value.h"

typedef struct {
    Token name;
    InstructionSet instructions;
} FunctionSymbol;

typedef struct {
    FunctionSymbol *items;
    usize count;
    usize capacity;
} FunctionTable;

typedef struct {
    InstructionSet instructions;
    s8Array globals;
    ValueArray constants;
    FunctionTable functions;
} ConversionResult;

ConversionResult convert(AstNode *program);

#endif