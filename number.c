#include "number.h"
#include <stdio.h>
#include "types.h"

static inline f64 _get_as_f64(Number n) {
    return (n.num_type == NUM_FLOAT) ? n.num_val.d : (f64)n.num_val.i;
}

static inline i32 _get_i32(Number n) {
    return n.num_val.i;
}

static inline f64 _get_f64(Number n) {
    return n.num_val.d;
}

static inline NumType _get_type(Number n) {
    return n.num_type;
}

static inline i32 _same_type(Number n1, Number n2, NumType t) {
    return _get_type(n1) == t && _get_type(n2) == t;
}

static inline i32 _both_i32(Number n1, Number n2) {
    return _same_type(n1, n2, NUM_INT);
}

Number new_num_int(i32 val) {
    Number n;
    n.num_type = NUM_INT;
    n.num_val.i = val;
    return n;
}

Number new_num_float(f64 val) {
    Number n;
    n.num_type = NUM_FLOAT;
    n.num_val.d = val;
    return n;
}

Number num_add(Number n1, Number n2) {
    return _both_i32(n1, n2) ? new_num_int(_get_i32(n1) + _get_i32(n2))
                             : new_num_float(_get_as_f64(n1) + _get_as_f64(n2));
}

Number num_sub(Number n1, Number n2) {
    return _both_i32(n1, n2) ? new_num_int(_get_i32(n1) - _get_i32(n2))
                             : new_num_float(_get_as_f64(n1) - _get_as_f64(n2));
}

Number num_mul(Number n1, Number n2) {
    return _both_i32(n1, n2) ? new_num_int(_get_i32(n1) * _get_i32(n2))
                             : new_num_float(_get_as_f64(n1) * _get_as_f64(n2));
}

Number num_div(Number n1, Number n2) {
    return _both_i32(n1, n2) ? new_num_int(_get_i32(n1) / _get_i32(n2))
                             : new_num_float(_get_as_f64(n1) / _get_as_f64(n2));
}

#define num_compute(n1, n2, oper) \
    (                             \
        Number _a = (n1),        \
        Number _b = (n2),        \
        _both_i32(_a, _b) ?       \
            new_num_int(_get_i32(_a) oper _get_i32(_b)) : \
            new_num_float(_get_as_f64(_a) oper _get_as_f64(_b)) \
    )

void print_num(Number n) {
    if (_get_type(n) == NUM_INT) {
        printf("i32: %d\n", _get_i32(n));
    } else {
        printf("float: %lf\n", _get_f64(n));
    }
}