#include "types.h"
#include "instructions.h"
#include "debug.h"
#include "ast/scanner.h"
#include <stdio.h>

static inline byte *_read_file(byte *path);

i32 main(void) {
    byte *source = _read_file("test.polo");
    ScanResult result = scan(source);

    if (result.error) {
        printf("Some error occured\n");
    } else {
        pretty_print_tokens(result.tokens);
    }

    return 0;
}

static inline 
byte *_read_file(byte *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(-1);
    }

    fseek(file, 0L, SEEK_END);
    usize file_size = ftell(file);
    rewind(file);

    byte *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(-1);
    }
    
    usize bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(-1);
    }
    
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}
