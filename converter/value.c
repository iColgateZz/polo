#include "value.h"
#include "macros.h"

Value new_val_bool(b32 val) {
    return (Value) {
        .type = VAL_BOOL,
        .bool = val
    };
}

Value new_val_num(Number val) {
    return (Value) {
        .type = VAL_NUM,
        .num = val
    };
}

Value new_val_str(s8 val) {
    return (Value) {
        .type = VAL_STR,
        .str = val
    };
}

void print_val(Value val) {
    switch (val.type) {
        case VAL_BOOL: {
            printf("%s", bool_str(val.bool));
            break;
        }
        case VAL_NUM: {
            print_num(val.num);
            break;
        }
        case VAL_STR: {
            printf("%.*s", (i32)val.str.len, val.str.s);
            break;
        }
        default: UNREACHABLE();
    }
    printf("\n");
}
