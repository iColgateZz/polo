#ifndef INSTRUCTIONS_INCLUDE
#define INSTRUCTIONS_INCLUDE

#include "types.h"
#include "da.h"

typedef enum {
    iReturn
} Instruction;

typedef struct {
    Instruction* items;
    usize count;
    usize capacity;
} InstructionSet;

#endif