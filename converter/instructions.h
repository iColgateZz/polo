#ifndef INSTRUCTIONS_INCLUDE
#define INSTRUCTIONS_INCLUDE

#include "types.h"

typedef enum {
    iPush_Num,
    iPush_Str,
    iPush_Bool,
    iPop,

    iStore_Global,
    iLoad_Global,
    iStore_Local,
    iLoad_Local,

    iAdd,
    iSub,
    iMul,
    iDiv,
    iNeg,

    iAnd,
    iOr,
    iNot,

    iEq,
    iNeq,
    iLt,
    iLte,
    iGt,
    iGte,

    iHalt,

    iReturn,
    iCall,
    iRestore,
    iSave
} Instruction;

typedef struct {
    Instruction* items;
    usize count;
    usize capacity;
} InstructionSet;

#endif