#include "types.h"
#include "instructions.h"
#include "debug.h"

i32 main(void) {
    InstructionSet set = {0};
    da_append(&set, iReturn);

    disassembleInstructionSet(set, "test data");
    
    return 0;
}
