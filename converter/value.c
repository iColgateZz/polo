#include "value.h"
#include "macros.h"
#include <stdlib.h>
#include <string.h>

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
    void *str = malloc(val.len);
    if (!str) UNREACHABLE();

    memcpy(str, val.s, val.len);
    return (Value) {
        .type = VAL_STR,
        .str = s8(str, val.len)
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
