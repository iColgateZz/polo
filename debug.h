#ifndef DEBUG_INCLUDE
#define DEBUG_INCLUDE

#include "types.h"
#include "instructions.h"

void disassembleInstructionSet(InstructionSet set, byte *set_name);
usize disassembleInstruction(InstructionSet set, usize offset);

#endif