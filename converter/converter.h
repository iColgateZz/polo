#ifndef CONVERTER_INCLUDE
#define CONVERTER_INCLUDE

#include "instructions.h"
#include "../ast/node.h"
#include "value.h"

typedef struct {
    InstructionSet instructions;
    s8Array debug_globals;
    ValueArray globals;
    s8Array debug_constants;
    ValueArray constants;
} ConversionResult;

ConversionResult convert(AstNode *program);

#endif