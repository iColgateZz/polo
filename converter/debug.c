#include "debug.h"
#include <stdio.h>
#include "s8.h"

#define UNREACHABLE()   \
        do {            \
            fprintf(stderr, "Unreachable: %s, %d\n", __FILE__, __LINE__);   \
            exit(-1);   \
        } while (0)

static inline usize _simple_instruction(byte *name, usize offset);
static inline usize _disassemble_instruction(ConversionResult result, usize offset);
static inline usize _double_instruction(Instruction i, usize offset, ConversionResult result);
static inline usize _const(Instruction i, usize offset, ConversionResult result);

#define instruction_size 1

void disassemble(ConversionResult result, byte *set_name) {
    printf("== %s ==\n", set_name);

    for (usize offset = 0; offset < result.instructions.count;)
        offset = _disassemble_instruction(result, offset);
}

static inline
usize _disassemble_instruction(ConversionResult result, usize offset) {
    printf("%04zu ", offset);

    Instruction instruction = result.instructions.items[offset];
    switch (instruction) {
        case iReturn:   return _simple_instruction("iReturn", offset);
        case iPop:      return _simple_instruction("iPop", offset);
        case iAdd:      return _simple_instruction("iAdd", offset);
        case iSub:      return _simple_instruction("iSub", offset);
        case iMul:      return _simple_instruction("iMul", offset);
        case iDiv:      return _simple_instruction("iDiv", offset);
        case iNeg:      return _simple_instruction("iNeg", offset);
        case iAnd:      return _simple_instruction("iAnd", offset);
        case iOr:       return _simple_instruction("iOr", offset);
        case iNot:      return _simple_instruction("iNot", offset);
        case iEq:       return _simple_instruction("iEq", offset);
        case iNeq:      return _simple_instruction("iNeq", offset);
        case iLt:       return _simple_instruction("iLt", offset);
        case iLte:      return _simple_instruction("iLte", offset);
        case iGt:       return _simple_instruction("iGt", offset);
        case iGte:      return _simple_instruction("iGte", offset);
        case iHalt:     return _simple_instruction("iHalt", offset);

        case iPush_Num:
        case iPush_Str:
        case iPush_Bool:
            return _const(instruction, offset, result);

        case iStore_Global:
        case iLoad_Global:
            return _double_instruction(instruction, offset, result);

        default: UNREACHABLE();
    }
}

static inline 
usize _simple_instruction(byte *name, usize offset) {
    printf("%s\n", name);
    return offset + instruction_size;
}

static inline
usize _const(Instruction i, usize offset, ConversionResult result) {
    usize idx = result.instructions.items[offset + instruction_size];
    s8 str = result.constants.items[idx];
    switch (i) {
        case iPush_Num:  printf("iPush_Num ");  break;
        case iPush_Str:  printf("iPush_Str ");  break;
        case iPush_Bool: printf("iPush_Bool "); break;
        default: UNREACHABLE();
    }

    printf("%.*s\n", (i32)str.len, str.s);
    return offset + 2 * instruction_size;
}

static inline
usize _double_instruction(Instruction i, usize offset, ConversionResult result) {
    usize idx = result.instructions.items[offset + instruction_size];
    s8 str = result.globals.items[idx];
    switch (i) {
        case iStore_Global: printf("iStore_Global "); break;
        case iLoad_Global:  printf("iLoad_Global ");  break;
        default: UNREACHABLE();
    }

    printf("%.*s\n", (i32)str.len, str.s);
    return offset + 2 * instruction_size;
}