#ifndef LINKER_INCLUDE
#define LINKER_INCLUDE

#include "instructions.h"
#include "value.h"
#include "converter.h"

typedef struct {
    InstructionSet instructions;
    ValueArray constants;
    usize first_instr;
    b32 error;
} LinkResult;

LinkResult link(ConversionResult);
void print_link(LinkResult res);

#endif