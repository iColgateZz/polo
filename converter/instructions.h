#ifndef INSTRUCTIONS_INCLUDE
#define INSTRUCTIONS_INCLUDE

#include "types.h"

typedef enum {
    iPush_Const,
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

    iCall,
    iRestore,
    iSave,

    iPrint,

    iJmpZ,
    iJmp
} Instruction;

typedef struct {
    Instruction* items;
    usize count;
    usize capacity;
} InstructionSet;

#endif