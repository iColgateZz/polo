#include "vm.h"
#include "value.h"
#include "number.h"
#include "da.h"
#include "macros.h"
#include <stdio.h>

typedef struct {
    usize *items;
    usize count;
    usize capacity;
} UsizeStack;

typedef struct {
    usize base_pointer;
    usize instr_pointer;
    ValueArray stack;
    ValueArray constants;
    ValueArray globals;
    ValueArray locals;
    UsizeStack return_stack;
    UsizeStack base_stack;
    UsizeStack top_stack;
} Vm;

static inline
void pushv(ValueArray *a, Value val) {
    da_append(a, val);
}

static inline
Value popv(ValueArray *a) {
    if (a->count > 0) {
        return a->items[--a->count];
    }
    UNREACHABLE();
}

static inline
void storev(ValueArray *a, usize idx, Value val) {
    a->items[idx] = val;
}

static inline
void pushu(UsizeStack *a, usize val) {
    da_append(a, val);
}

static inline
usize popu(UsizeStack *a) {
    if (a->count > 0) {
        return a->items[--a->count];
    }
    UNREACHABLE();
}

static inline
void storeu(UsizeStack *a, usize idx, usize val) {
    a->items[idx] = val;
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

    da_reserve(&vm.globals, 256);
    da_reserve(&vm.locals, 256);

    InstructionSet instructions = res.instructions;

    while (true) {
        // printf("Instr_ptr: %zu\n", vm.instr_pointer);
        Instruction instr = get_instr(instructions, vm.instr_pointer++);

        switch (instr) {

            case iHalt:
                return true;

            case iPush_Const: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                pushv(&vm.stack, vm.constants.items[idx]);
                break;
            }

            case iPop: {
                popv(&vm.stack);
                break;
            }

            case iStore_Global: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                vm.globals.items[idx] = popv(&vm.stack);
                break;
            }

            case iLoad_Global: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                pushv(&vm.stack, vm.globals.items[idx]);
                break;
            }

            case iStore_Local: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                storev(&vm.locals, vm.base_pointer + idx, popv(&vm.stack));
                break;
            }

            case iLoad_Local: {
                usize idx = get_instr(instructions, vm.instr_pointer++);
                pushv(&vm.stack, vm.locals.items[vm.base_pointer + idx]);
                break;
            }

            case iAdd: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_num(num_add(a.num, b.num)));
                break;
            }

            case iSub: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_num(num_sub(a.num, b.num)));
                break;
            }

            case iMul: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_num(num_mul(a.num, b.num)));
                break;
            }

            case iDiv: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_num(num_div(a.num, b.num)));
                break;
            }

            case iNeg: {
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_num(num_mul(a.num, new_num_int(-1))));
                break;
            }

            case iAnd: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_bool(a.bool && b.bool));
                break;
            }

            case iOr: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_bool(a.bool || b.bool));
                break;
            }

            case iNot: {
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_bool(!a.bool));
                break;
            }

            case iEq: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                if (a.type == VAL_NUM)
                    pushv(&vm.stack, new_val_bool(num_eq(a.num, b.num)));
                else
                    pushv(&vm.stack, new_val_bool(a.bool == b.bool));
                break;
            }

            case iNeq: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                if (a.type == VAL_NUM)
                    pushv(&vm.stack, new_val_bool(!num_eq(a.num, b.num)));
                else
                    pushv(&vm.stack, new_val_bool(a.bool != b.bool));
                break;
            }

            case iLt: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_bool(num_lt(a.num, b.num)));
                break;
            }

            case iLte: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_bool(num_lte(a.num, b.num)));
                break;
            }

            case iGt: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_bool(num_gt(a.num, b.num)));
                break;
            }

            case iGte: {
                Value b = popv(&vm.stack);
                Value a = popv(&vm.stack);
                pushv(&vm.stack, new_val_bool(num_gte(a.num, b.num)));
                break;
            }

            case iSave: {
                pushu(&vm.base_stack, vm.base_pointer);
                vm.base_pointer = vm.locals.count;
                pushu(&vm.top_stack, vm.stack.count);
                break;
            }

            case iRestore: {
                if (vm.return_stack.count == 0)
                    return false;

                vm.instr_pointer = vm.return_stack.items[--vm.return_stack.count];

                vm.base_pointer = popu(&vm.base_stack);
                // usize prev_top_stack = popu(&vm.top_stack);
                
                // if (vm.stack.count > prev_top_stack) {
                //     Value ret_val = popv(&vm.stack);
                //     vm.stack.count = prev_top_stack;
                //     pushv(&vm.stack, ret_val);
                // } else {
                //     vm.stack.count = prev_top_stack;
                // }

                break;
            }

            case iCall: {
                /* Compute how many argument values were pushed after the last iSave.
                iSave pushed the previous operand-stack size onto top_stack, so
                the last entry of top_stack is the operand stack size before
                arguments were pushed. */
                usize addr = get_instr(instructions, vm.instr_pointer++);

                if (vm.top_stack.count == 0) {
                    /* No saved top — calling without prior iSave is an error; handle as you prefer. */
                    return false;
                }

                usize prev_stack_count = vm.top_stack.items[vm.top_stack.count - 1];
                if (vm.stack.count < prev_stack_count) {
                    /* Shouldn't happen: stack underflow relative to saved top */
                    return false;
                }

                usize num_args = vm.stack.count - prev_stack_count;

                /* Move args from operand stack into new locals at base_pointer + i.
                Arguments order must match the compiler's order. Here we copy them
                such that the oldest argument is at locals[base + 0], and the last
                pushed argument is at locals[base + num_args-1]. */
                for (usize i = 0; i < num_args; ++i) {
                    /* source index in operand stack */
                    usize src = prev_stack_count + i;
                    /* destination local index */
                    storev(&vm.locals, vm.base_pointer + i, vm.stack.items[src]);
                }

                /* drop arguments from operand stack (reset to prev saved top) */
                vm.stack.count = prev_stack_count;

                /* save return address and jump */
                pushu(&vm.return_stack, vm.instr_pointer);
                vm.instr_pointer = addr;
                break;
            }


            case iPrint: {
                Value val = popv(&vm.stack);
                print_val(val);
                break;
            }

            default: UNREACHABLE();
        }
    }
}
