#ifndef VALUE_INCLUDE
#define VALUE_INCLUDE

#include "types.h"
#include "number.h"
#include "s8.h"

typedef enum {
    VAL_BOOL,
    VAL_NUM,
    VAL_STR
} ValueType;

typedef struct {
    ValueType type;
    union {
        b32 bool;
        Number num;
        s8 str;
    };
} Value;

typedef struct {
    Value *items;
    usize count;
    usize capacity;
} ValueArray;

Value new_val_bool(b32 val);
Value new_val_num(Number val);
Value new_val_str(s8 val);

#endif