#ifndef SCANNER_INCLUDE
#define SCANNER_INCLUDE

#include "types.h"
#include "token.h"

typedef struct {
    TokenArray tokens;
    b32 error;
} ScanResult;

ScanResult scan(byte *path);
void pretty_print_tokens(TokenArray tokens);

#endif