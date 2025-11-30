#ifndef CONVERTER_INCLUDE
#define CONVERTER_INCLUDE

#include "instructions.h"
#include "../ast/node.h"
#include "value.h"

typedef struct {
    InstructionSet instructions;
    s8Array debug_globals;
    ValueArray constants;
} ConversionResult;

ConversionResult convert(AstNode *program);

#endif