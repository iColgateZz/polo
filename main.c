#include "types.h"
#include "ast/scanner.h"
#include "ast/parser.h"
#include "ast/ast_printer.h"
#include "ast/ast_checker.h"
#include "converter/converter.h"
#include "converter/debug.h"
#include <stdio.h>
#include <stdlib.h>

static inline byte *_read_file(byte *path);

i32 main(void) {
    byte *source = _read_file("test.polo");

    ScanResult scan_result = scan(source);
    if (scan_result.error) {
        return -1;
    }

    pretty_print_tokens(scan_result.tokens);

    ParseResult parse_result = parse(scan_result.tokens);
    if (parse_result.error) {
        return -1;
    }

    print_ast(parse_result.program, 0);

    if (semantic_errors(parse_result.program)) {
        return -1;
    }

    ConversionResult conv_result = convert(parse_result.program);
    disassemble(conv_result, "main");

    // free(source);
    // free(scan_result.tokens.items);
    // free_ast(parse_result.program);

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
