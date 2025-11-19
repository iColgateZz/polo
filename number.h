#ifndef Number_H
#define Number_H

#include "types.h"

typedef enum {
    NUM_INT,
    NUM_FLOAT
} NumType;

typedef union {
    i32 i;
    f64 d;
} NumVal;

typedef struct Number {
    NumType num_type;
    NumVal num_val;
} Number;

// Implemented
Number new_num_int(i32 val);
Number new_num_float(f64 val);

Number num_add(Number n1, Number n2);
Number num_sub(Number n1, Number n2);
Number num_mul(Number n1, Number n2);
Number num_div(Number n1, Number n2);

void pri32_num(Number n);

#endif