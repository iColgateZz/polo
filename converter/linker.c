#include "linker.h"
#include "da.h"
#include "types.h"
#include "macros.h"
#include "s8.h"
#include <stdio.h>
#include <stdarg.h>

typedef struct {
    usize *items;
    usize count;
    usize capacity;
    usize cur;
} Queue;

void enqueue(Queue *q, usize i) {
    da_append(q, i);
}

usize deque(Queue *q) {
    if (q->cur < q->count) {
        return q->items[q->cur++];
    }
    UNREACHABLE();
}

usize peek(Queue *q) {
    return q->items[q->cur];
}

b32 is_empty(Queue *q) {
    return q->cur == q->count;
}

typedef struct {
    b32 in_use;
    usize address;
} Entry;

typedef struct {
    Entry *items;
    usize count;
    usize capacity;
} EntryArr;

b32 used(EntryArr arr, usize idx) {
    return arr.items[idx].in_use;
}

void mark_used(EntryArr arr, usize idx) {
    arr.items[idx].in_use = true;
}

void set_address(EntryArr arr, usize idx, usize address) {
    arr.items[idx].address = address;
}

usize get_address(EntryArr arr, usize idx) {
    return arr.items[idx].address;
}

static inline
void _linker_error(const byte *fmt, ...) {
    fprintf(stderr, "Linker error: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

static inline
b32 _s8_eq(s8 s1, s8 s2) {
    return s1.len == s2.len &&
        memcmp(s1.s, s2.s, s1.len) == 0;
}

b32 has_arg(Instruction instr) {
    return instr == iPush_Num || instr == iPush_Str || 
           instr == iPush_Bool || instr == iStore_Global ||
           instr == iLoad_Global || instr == iStore_Local ||
           instr == iLoad_Local;
}

static usize dfs_link_function(
    usize fn_idx,
    ConversionResult *conv,
    EntryArr *use_arr,
    InstructionSet *instructions,
    usize write_head
) {
    if (used(*use_arr, fn_idx)) {
        return get_address(*use_arr, fn_idx);
    }

    FunctionSymbol *fn = &conv->functions.items[fn_idx];
    usize start_address = write_head;
    
    mark_used(*use_arr, fn_idx);
    set_address(*use_arr, fn_idx, start_address);

    usize i = 0;
    usize to_ret = 0;
    while (i < fn->instructions.count) {
        Instruction instr = fn->instructions.items[i];
        instructions->items[write_head++] = instr;

        if (instr == iCall) {
            // Next instruction is the callee index
            ++i;
            usize callee_idx = fn->instructions.items[i];

            // Recursively link the callee, get its start address
            usize callee_addr = dfs_link_function(
                callee_idx, conv, use_arr, instructions, write_head
            );

            if (callee_addr > to_ret) 
                to_ret = callee_addr;

            // Write the callee's address as the operand
            instructions->items[write_head++] = callee_addr;
        } else if (has_arg(instr)) {
            ++i;
            instructions->items[write_head++] = fn->instructions.items[i];
        }
        ++i;
    }

    // Return the new write_head (end address after this function)
    return write_head > to_ret ? write_head : to_ret;
}

LinkResult link(ConversionResult conv) {
    EntryArr use_arr = {0};
    da_reserve(&use_arr, conv.functions.count);

    usize instructions_len = 0;
    for (usize i = 0; i < conv.functions.count; ++i) {
        use_arr.items[i] = (Entry) { .in_use = false };
        instructions_len += conv.functions.items[i].instructions.count;

        if (conv.functions.items[i].instructions.count == 0) {
            _linker_error("function's '%.*s' body not provided. "
                          "Prototype mentioned at line %d",
                (i32)conv.functions.items[i].name.str.len, 
                conv.functions.items[i].name.str.s, 
                conv.functions.items[i].name.line);
            return (LinkResult) { .error = true };
        }
    }

    InstructionSet instructions = {0};
    da_reserve(&instructions, instructions_len);

    b32 main_found = false;
    usize main_idx;
    for (usize i = 0; i < conv.functions.count; ++i) {
        if (_s8_eq(conv.functions.items[i].name.str, s8("main"))) {
            if (!main_found) {
                main_found = true;
                main_idx = i;
            } else {
                _linker_error("mulitple 'main' functions found");
                return (LinkResult) { .error = true };
            }
        }
    }

    if (!main_found) {
        _linker_error("function 'main' not found");
        return (LinkResult) { .error = true };
    }

    // Code from functions
    for (usize i = 0; i < conv.functions.count; ++i) {
        usize last = dfs_link_function(i, &conv, &use_arr, &instructions, instructions.count);
        instructions.count = last;
    }

    usize first_instr = instructions.count;

    // Code to run before calling 'main'
    for (usize i = 0; i < conv.instructions.count; ++i) {
        Instruction instr = conv.instructions.items[i];
        da_append(&instructions, instr);

        if (instr == iCall) {
            usize callee_idx = conv.instructions.items[++i];
            usize callee_addr = get_address(use_arr, callee_idx);
            da_append(&instructions, callee_addr);
        } else if (has_arg(instr)) {
            da_append(&instructions, conv.instructions.items[++i]);
        }
    }

    // Override iHalt
    instructions.items[instructions.count - 1] = iSave;
    // Call main
    da_append(&instructions, iCall);
    da_append(&instructions, get_address(use_arr, main_idx));
    da_append(&instructions, iHalt);

    return (LinkResult) { 
        .constants = conv.constants, 
        .instructions = instructions, 
        .first_instr = first_instr 
    };
}

void _print_instr(Instruction instr) {
    switch (instr) {
    case iPush_Num:      printf("iPush_Num");      break;
    case iPush_Str:      printf("iPush_Str");      break;
    case iPush_Bool:     printf("iPush_Bool");     break;
    case iPop:           printf("iPop");           break;

    case iStore_Global:  printf("iStore_Global");  break;
    case iLoad_Global:   printf("iLoad_Global");   break;
    case iStore_Local:   printf("iStore_Local");   break;
    case iLoad_Local:    printf("iLoad_Local");    break;

    case iAdd:           printf("iAdd");           break;
    case iSub:           printf("iSub");           break;
    case iMul:           printf("iMul");           break;
    case iDiv:           printf("iDiv");           break;
    case iNeg:           printf("iNeg");           break;

    case iAnd:           printf("iAnd");           break;
    case iOr:            printf("iOr");            break;
    case iNot:           printf("iNot");           break;

    case iEq:            printf("iEq");            break;
    case iNeq:           printf("iNeq");           break;
    case iLt:            printf("iLt");            break;
    case iLte:           printf("iLte");           break;
    case iGt:            printf("iGt");            break;
    case iGte:           printf("iGte");           break;

    case iHalt:          printf("iHalt");          break;

    case iReturn:        printf("iReturn");        break;
    case iCall:          printf("iCall");          break;
    case iRestore:       printf("iRestore");       break;
    case iSave:          printf("iSave");          break;

    default:
        printf("UNKNOWN_INSTRUCTION");
        break;
    }
}

void print_link(LinkResult res) {
    printf("== After linking ==\n");
    printf("== Start: %04zu ==\n", res.first_instr);

    for (usize i = 0; i < res.instructions.count; ++i) {
        Instruction instr = res.instructions.items[i];
        printf("%04zu ", i);
        _print_instr(instr);

        if (instr == iCall || has_arg(instr)) {
            usize arg = res.instructions.items[++i];
            printf(" %zu", arg);
        }
        printf("\n");
    }
}
