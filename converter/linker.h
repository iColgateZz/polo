#ifndef LINKER_INCLUDE
#define LINKER_INCLUDE

#include "instructions.h"
#include "value.h"
#include "converter.h"

typedef struct {
    InstructionSet instructions;
    ValueArray constants;
    b32 error;
} LinkResult;

LinkResult link(ConversionResult);

#endif