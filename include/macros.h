#ifndef MACROS_INCLUDE
#define MACROS_INCLUDE

#include <stddef.h>
#include <stdio.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define return_defer(value) do { result = (value); goto defer; } while(0)

#define UNREACHABLE()   \
        do {            \
            fprintf(stderr, "Unreachable: %s, %d\n", __FILE__, __LINE__);   \
            exit(-1);   \
        } while (0)

#define bool_str(i) ((i) ? "true" : "false")

#endif