#ifndef CONVERTER_INCLUDE
#define CONVERTER_INCLUDE

#include "types.h"
#include "instructions.h"
#include "../ast/node.h"

typedef struct {
    InstructionSet instructions;
    b32 error;
} ConversionResult;

ConversionResult convert(AstNode *program);

#endif