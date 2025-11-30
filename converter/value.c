#include "value.h"

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
