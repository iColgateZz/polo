#include "vm.h"
#include "value.h"
#include "number.h"
#include "da.h"
#include "macros.h"

typedef struct {
    usize *items;
    usize count;
    usize capacity;
} ReturnAddrStack;

typedef struct {
    usize base_pointer;
    usize instr_pointer;
    ValueArray stack;
    ValueArray constants;
    ValueArray globals;
    ReturnAddrStack return_stack;
} Vm;

static inline
void push(ValueArray *a, Value val) {
    da_append(a, val);
}

static inline
Value pop(ValueArray *a) {
    if (a->count > 0) {
        return a->items[--a->count];
    }
    UNREACHABLE();
}

static inline 
Value peek(ValueArray *a) {
    if (a->count > 0) {
        return a->items[a->count - 1];
    }
    UNREACHABLE();
}

static inline
Instruction get_instr(InstructionSet i, usize idx) {
    return i.items[idx];
}

b32 run(LinkResult res) {
    Vm vm = { 
        .instr_pointer = res.first_instr,
        .constants = res.constants
    };

    InstructionSet instructions = res.instructions;
    Value ret_val;
    b32 has_ret = false;

    while (true) {
        Instruction instr = get_instr(instructions, vm.instr_pointer++);

        switch (instr) {

            case iHalt:
                return true;

            case iPush_Str:
            case iPush_Bool:
            case iPush_Num: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                push(&vm.stack, vm.constants.items[idx]);
                break;
            }

            case iPop: {
                pop(&vm.stack);
                break;
            }

            case iStore_Global: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                vm.globals.items[idx] = pop(&vm.stack);
                break;
            }

            case iLoad_Global: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                push(&vm.stack, vm.globals.items[idx]);
                break;
            }

            case iStore_Local: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                vm.stack.items[vm.base_pointer + idx] = pop(&vm.stack);
                break;
            }

            case iLoad_Local: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                push(&vm.stack, vm.stack.items[vm.base_pointer + idx]);
                break;
            }

            case iAdd: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_num(num_add(a.num, b.num)));
                break;
            }

            case iSub: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_num(num_sub(a.num, b.num)));
                break;
            }

            case iMul: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_num(num_mul(a.num, b.num)));
                break;
            }

            case iDiv: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_num(num_div(a.num, b.num)));
                break;
            }

            case iNeg: {
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_num(num_mul(a.num, new_num_int(-1))));
                break;
            }

            case iAnd: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_bool(a.bool && b.bool));
                break;
            }

            case iOr: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_bool(a.bool || b.bool));
                break;
            }

            case iNot: {
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_bool(!a.bool));
                break;
            }

            case iEq: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                if (a.type == VAL_NUM)
                    push(&vm.stack, new_val_bool(num_eq(a.num, b.num)));
                else
                    push(&vm.stack, new_val_bool(a.bool == b.bool));
                break;
            }

            case iNeq: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                if (a.type == VAL_NUM)
                    push(&vm.stack, new_val_bool(!num_eq(a.num, b.num)));
                else
                    push(&vm.stack, new_val_bool(a.bool != b.bool));
                break;
            }

            case iLt: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_bool(num_lt(a.num, b.num)));
                break;
            }

            case iLte: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_bool(num_lte(a.num, b.num)));
                break;
            }

            case iGt: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_bool(num_gt(a.num, b.num)));
                break;
            }

            case iGte: {
                Value b = pop(&vm.stack);
                Value a = pop(&vm.stack);
                push(&vm.stack, new_val_bool(num_gte(a.num, b.num)));
                break;
            }

            case iSave: {
                Value bp_val = new_val_num(new_num_int((i32)vm.base_pointer));
                push(&vm.stack, bp_val);
                vm.base_pointer = vm.stack.count;
                break;
            }

            case iRestore: {
                // restore instruction pointer
                if (vm.return_stack.count == 0) 
                    return false;
                vm.instr_pointer = vm.return_stack.items[--vm.return_stack.count];

                // restore base pointer
                if (vm.base_pointer == 0)
                    return false;
                // previous base pointer is stored just before the current frame
                Value bp_val = vm.stack.items[vm.base_pointer - 1];
                usize prev_bp = bp_val.num.num_val.i;
                // pop everything above the previous base pointer (including the saved bp)
                vm.stack.count = vm.base_pointer - 1;
                vm.base_pointer = prev_bp;

                if (has_ret) {
                    push(&vm.stack, ret_val);
                    has_ret = false;
                }

                break;
            }

            case iCall: {
                usize addr = get_instr(instructions, vm.instr_pointer++);
                da_append(&vm.return_stack, vm.instr_pointer);
                vm.instr_pointer = addr;
                break;
            }

            case iReturn: {
                ret_val = pop(&vm.stack);
                has_ret = true;
                break;
            }

            default: UNREACHABLE();
        }
    }
}
