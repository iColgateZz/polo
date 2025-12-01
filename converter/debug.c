#include "debug.h"
#include <stdio.h>
#include "s8.h"
#include "macros.h"

static inline usize _simple_instruction(byte *name, usize offset);
static inline usize _disassemble_instruction(InstructionSet *instructions, ConversionResult result, usize offset);
static inline usize _global_instruction(Instruction i, usize offset, ConversionResult result, InstructionSet *instructions);
static inline usize _const(usize offset, ConversionResult result, InstructionSet *instructions);
static inline usize _local_instruction(Instruction i, usize offset, InstructionSet *instructions);
static inline usize _call_instruction(usize offset, ConversionResult result, InstructionSet *instructions);

#define instruction_size 1

static void _disassemble_set(InstructionSet *instructions, ConversionResult result, const char *set_name) {
    printf("== %s ==\n", set_name);

    for (usize offset = 0; offset < instructions->count;)
        offset = _disassemble_instruction(instructions, result, offset);
}

void disassemble(ConversionResult result, byte *set_name) {
    _disassemble_set(&result.instructions, result, (const char *)set_name);

    for (usize i = 0; i < result.functions.count; ++i) {
        FunctionSymbol *fn = &result.functions.items[i];
        char fn_label[256];
        snprintf(fn_label, sizeof(fn_label), "function %.*s", (int)fn->name.str.len, fn->name.str.s);
        _disassemble_set(&fn->instructions, result, fn_label);
    }
}

static inline
usize _disassemble_instruction(InstructionSet *instructions, ConversionResult result, usize offset) {
    printf("%04zu ", offset);
    Instruction instruction = instructions->items[offset];
    switch (instruction) {
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

        case iPrint:    return _simple_instruction("iPrint", offset);

        case iSave:     return _simple_instruction("iSave", offset);
        case iRestore:  return _simple_instruction("iRestore", offset);

        case iPop:      return _simple_instruction("iPop", offset);
        case iPush_Const:
            return _const(offset, result, instructions);

        case iStore_Global:
        case iLoad_Global:
            return _global_instruction(instruction, offset, result, instructions);

        case iStore_Local:
        case iLoad_Local:
        case iJmp:
        case iJmpZ:
            return _local_instruction(instruction, offset, instructions);

        case iCall:
            return _call_instruction(offset, result, instructions);

        default: UNREACHABLE();
    }
}

static inline 
usize _simple_instruction(byte *name, usize offset) {
    printf("%s\n", name);
    return offset + instruction_size;
}

static inline
usize _const(usize offset, ConversionResult result, InstructionSet *instructions) {
    usize idx = instructions->items[offset + instruction_size];
    Value value = result.constants.items[idx];
    printf("iPush_Const ");

    switch (value.type) {
        case VAL_BOOL:
            printf("%s\n", bool_str(value.bool));
            break;
        case VAL_STR:
            printf("%.*s\n", (i32)value.str.len, value.str.s);
            break;
        case VAL_NUM:
            print_num(value.num);
            printf("\n");
            break;
        
        default: UNREACHABLE();
    }
    return offset + 2 * instruction_size;
}

static inline
usize _global_instruction(Instruction i, usize offset, ConversionResult result, InstructionSet *instructions) {
    usize idx = instructions->items[offset + instruction_size];
    s8 str = result.globals.items[idx];
    switch (i) {
        case iStore_Global: printf("iStore_Global "); break;
        case iLoad_Global:  printf("iLoad_Global ");  break;
        default: UNREACHABLE();
    }

    printf("%.*s\n", (i32)str.len, str.s);
    return offset + 2 * instruction_size;
}

static inline
usize _local_instruction(Instruction i, usize offset, InstructionSet *instructions) {
    isize idx = instructions->items[offset + instruction_size];
    switch (i) {
        case iStore_Local: printf("iStore_Local "); break;
        case iLoad_Local:  printf("iLoad_Local ");  break;
        case iJmp:         printf("iJmp ");         break;
        case iJmpZ:        printf("iJmpZ ");        break;
        default: UNREACHABLE();
    }

    printf("%ld\n", idx);
    return offset + 2 * instruction_size;
}

static inline
usize _call_instruction(usize offset, ConversionResult result, InstructionSet *instructions) {
    usize idx = instructions->items[offset + instruction_size];
    s8 fn_name = result.functions.items[idx].name.str;
    printf("iCall %.*s\n", (i32)fn_name.len, fn_name.s);
    return offset + 2 * instruction_size;
}