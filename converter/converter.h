#ifndef CONVERTER_INCLUDE
#define CONVERTER_INCLUDE

#include "types.h"
#include "instructions.h"
#include "../ast/node.h"
#include "s8.h"

typedef struct {
    InstructionSet instructions;
    s8Array globals;
    s8Array constants;
} ConversionResult;

ConversionResult convert(AstNode *program);

#endif