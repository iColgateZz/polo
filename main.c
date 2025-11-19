#include "types.h"
#include "instructions.h"
#include "debug.h"
#include "scanner.h"
#include <stdio.h>


i32 main(void) {
    ScanResult result = scan("test.polo");

    if (result.error) {
        printf("Some error occured\n");
    } else {
        pretty_print_tokens(result.tokens);
    }

    return 0;
}
