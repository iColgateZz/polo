#include "debug.h"
#include <stdio.h>

static inline usize _simple_instruction(byte *name, usize offset);
static inline usize _disassemble_instruction(InstructionSet set, usize offset);

void disassemble(InstructionSet set, byte *set_name) {
    printf("== %s ==\n", set_name);

    for (usize offset = 0; offset < set.count;)
        offset = _disassemble_instruction(set, offset);
}

static inline
usize _disassemble_instruction(InstructionSet set, usize offset) {
    printf("%04zu ", offset);

    Instruction instruction = set.items[offset];
    switch (instruction) {
        case iReturn:
            return _simple_instruction("iReturn", offset);
        default:
            printf("Unknown instruction %d\n", instruction);
            return offset + 1;
    }
}

static inline 
usize _simple_instruction(byte *name, usize offset) {
    printf("%s\n", name);
    return offset + 1;
}