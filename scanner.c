#include "scanner.h"
#include <stdio.h>
#include <string.h>
#include "da.h"

typedef struct Scanner {
    byte *start;
    byte *current;
    u32 line;
    b32 error;
} Scanner;

static Scanner scanner;

static inline
void _init_scanner(byte *source) {
    scanner.current = source;
    scanner.start = source;
    scanner.line = 1;
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

static inline 
b32 _is_alpha(byte c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static inline 
b32 _is_digit(byte c) {
    return c >= '0' && c <= '9';
}

static inline 
b32 _is_at_end(void) {
    return *scanner.current == '\0';
}

static inline 
byte _advance(void) {
    scanner.current++;
    return scanner.current[-1];
}

static inline 
byte _peek(void) {
    return *scanner.current;
}

static inline 
byte _peek_next(void) {
    if (_is_at_end()) return '\0';
    return scanner.current[1];
}

static inline 
b32 _match(byte expected) {
    if (_is_at_end()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static inline
Token _new_token(TokenType type) {
    return (Token) {
        .line = scanner.line,
        .type = type,
        .str = s8(scanner.start, scanner.current - scanner.start)
    };
}

static inline
Token _error_token(byte *msg) {
    scanner.error = true;
    return (Token) {
        .line = scanner.line,
        .type = TOKEN_ERROR,
        .str = s8(msg, scanner.current - scanner.start)
    };
}

static inline
void _skip_whitespace(void) {
    for (;;) {
        byte c = _peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            _advance();
            break;
        case '\n':
            scanner.line++;
            _advance();
            break;
        case '/':
            if (_peek_next() == '/') {
                while (_peek() != '\n' && !_is_at_end()) _advance();
            } else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

static inline 
TokenType _check_keyword(i32 start, i32 length, const byte *rest, TokenType type) {
    if ((scanner.current - scanner.start) == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER_LITERAL;
}

static inline 
TokenType _identifier_type(void) {
    isize len = scanner.current - scanner.start;
    switch (scanner.start[0]) {
        case 'a': return _check_keyword(1, 2, "nd", TOKEN_AND);
        case 'b': 
            if (len > 1) {
                switch (scanner.start[1]) {
                    case 'o': return _check_keyword(2, 2, "ol", TOKEN_BOOL);
                    case 'r': return _check_keyword(2, 3, "eak", TOKEN_BREAK);
                }
            }
            break;
        case 'c': return _check_keyword(1, 7, "ontinue", TOKEN_CONTINUE);
        case 'e':
            if (len > 2) {
                switch (scanner.start[1]) {
                    case 'l':
                        if (scanner.start[2] == 'i')
                            return _check_keyword(3, 1, "f", TOKEN_ELIF);
                        else
                            return _check_keyword(2, 2, "se", TOKEN_ELSE);
                }
            }
            break;
        case 'f':
            if (len > 1) {
                switch (scanner.start[1]) {
                    case 'a': return _check_keyword(2, 3, "lse", TOKEN_FALSE_LITERAL);
                    case 'n': return _check_keyword(2, 0, "", TOKEN_FN);
                    case 'o': return _check_keyword(2, 1, "r", TOKEN_FOR);
                }
            }
            break;
        case 'i':
            if (len > 1) {
                switch (scanner.start[1]) {
                    case 'm': return _check_keyword(2, 4, "port", TOKEN_IMPORT);
                    case 'f': return _check_keyword(2, 0, "", TOKEN_IF);
                }
            }
            break;
        case 'n':
            if ((len) == 3)
                return _check_keyword(1, 2, "um", TOKEN_NUM);
            else if ((len) == 4)
                return _check_keyword(1, 3, "ull", TOKEN_NULL_LITERAL);
            break;
        case 'o': return _check_keyword(1, 1, "r", TOKEN_OR);
        case 'p': return _check_keyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return _check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': 
            if (len > 3 && memcmp(scanner.start, "str", 3) == 0) {
                switch (scanner.start[3]) {
                    case 'u': return _check_keyword(4, 2, "ct", TOKEN_STRUCT);
                    case 'i': return _check_keyword(4, 2, "ng", TOKEN_STRING);
                }
            }
            break;
        case 't': return _check_keyword(1, 3, "rue", TOKEN_TRUE_LITERAL);
        case 'v': return _check_keyword(1, 3, "oid", TOKEN_VOID);
        case 'w': return _check_keyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER_LITERAL;
}

static inline Token _identifier(void) {
    while (_is_alpha(_peek()) || _is_digit(_peek())) _advance();
    return _new_token(_identifier_type());
}

static inline Token _number(void) {
    while (_is_digit(_peek())) _advance();
    if (_peek() == '.' && _is_digit(_peek_next())) {
        _advance();
        while (_is_digit(_peek())) _advance();
    }
    return _new_token(TOKEN_NUMBER_LITERAL);
}

static inline Token _string(void) {
    while (_peek() != '"' && !_is_at_end()) {
        if (_peek() == '\n') scanner.line++;
        _advance();
    }
    if (_is_at_end()) return _error_token("Unterminated string");

    _advance(); // Consume closing quote
    return _new_token(TOKEN_STRING_LITERAL);
}

static inline
Token _scanToken(void) {
    _skip_whitespace();
    scanner.start = scanner.current;

    if (_is_at_end()) return _new_token(TOKEN_EOF);

    byte c = _advance();
    if (_is_alpha(c)) return _identifier();
    if (_is_digit(c)) return _number();

    switch (c) {
        case '(': return _new_token(TOKEN_LEFT_PAREN);
        case ')': return _new_token(TOKEN_RIGHT_PAREN);
        case '[': return _new_token(TOKEN_LEFT_BRACKET);
        case ']': return _new_token(TOKEN_RIGHT_BRACKET);
        case '{': return _new_token(TOKEN_LEFT_BRACE);
        case '}': return _new_token(TOKEN_RIGHT_BRACE);
        case ';': return _new_token(TOKEN_SEMICOLON);
        case ',': return _new_token(TOKEN_COMMA);
        case '.': return _new_token(TOKEN_DOT);
        case '-': return _new_token(TOKEN_MINUS);
        case '+': return _new_token(TOKEN_PLUS);
        case '/': return _new_token(TOKEN_SLASH);
        case '*': return _new_token(TOKEN_STAR);
        case '!': return _new_token(_match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return _new_token(_match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return _new_token(_match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return _new_token(_match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return _string();
  }

  return _error_token("Unexpected character");
}

ScanResult scan(byte *path) {
    byte *source = _read_file(path);
    _init_scanner(source);

    TokenArray tokens = {0};
    while (true) {
        Token token = _scanToken();
        da_append(&tokens, token);
        if (token.type == TOKEN_EOF) break;
    }

    return (ScanResult) {
        .tokens = tokens,
        .error = scanner.error
    };
}

static inline
byte *_token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_DOT: return "DOT";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_STAR: return "STAR";
        case TOKEN_BANG: return "BANG";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_LESS: return "LESS";
        case TOKEN_BANG_EQUAL: return "BANG_EQUAL";
        case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_IDENTIFIER_LITERAL: return "IDENTIFIER";
        case TOKEN_NUMBER_LITERAL: return "NUMBER";
        case TOKEN_STRING_LITERAL: return "STRING";
        case TOKEN_TRUE_LITERAL: return "TRUE";
        case TOKEN_FALSE_LITERAL: return "FALSE";
        case TOKEN_NULL_LITERAL: return "NULL";
        case TOKEN_NUM: return "NUM_TYPE";
        case TOKEN_STRING: return "STRING_TYPE";
        case TOKEN_BOOL: return "BOOL_TYPE";
        case TOKEN_VOID: return "VOID_TYPE";
        case TOKEN_FN: return "FN_TYPE";
        case TOKEN_STRUCT: return "STRUCT_TYPE";
        case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_IF: return "IF";
        case TOKEN_ELIF: return "ELIF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void pretty_print_tokens(TokenArray tokens) {
    if (tokens.count == 0) return;

    u32 current_line = tokens.items[0].line;
    printf("%u: ", current_line);

    // Print the first line's lexemes
    for (usize i = 0; i < tokens.count; ++i) {
        if (tokens.items[i].line != current_line) break;
        fwrite(tokens.items[i].str.s, 1, tokens.items[i].str.len, stdout);
        putchar(' ');
    }
    putchar('\n');

    // Print the first line's token types
    printf("%u: ", current_line);
    for (usize i = 0; i < tokens.count; ++i) {
        if (tokens.items[i].line != current_line) break;
        printf("%s ", _token_type_to_str(tokens.items[i].type));
    }
    putchar('\n');

    // Print subsequent lines
    for (usize i = 0; i < tokens.count;) {
        u32 line = tokens.items[i].line;
        if (line == current_line) {
            // Already printed above
            while (i < tokens.count && tokens.items[i].line == current_line) ++i;
            continue;
        }
        current_line = line;
        printf("%u: ", current_line);
        usize j = i;
        while (j < tokens.count && tokens.items[j].line == current_line) {
            fwrite(tokens.items[j].str.s, 1, tokens.items[j].str.len, stdout);
            putchar(' ');
            ++j;
        }
        putchar('\n');
        printf("%u: ", current_line);
        j = i;
        while (j < tokens.count && tokens.items[j].line == current_line) {
            printf("%s ", _token_type_to_str(tokens.items[j].type));
            ++j;
        }
        putchar('\n');
        i = j;
    }
}