# Polo Programming Language

Polo is a general-purpose programming language designed with simplicity and expressiveness in mind. It offers a clean syntax with static typing, allowing developers to write concise yet readable code. Polo is built with a complete compiler pipeline from lexical analysis to bytecode execution.

## Building the Project

To compile the Polo compiler and VM, use the standard make command:

```bash
make
```

## Running Code

To run code in file.polo, use the following command:

```bash
./polo file.polo
```

## Language Overview

Polo is a statically typed language with a straightforward syntax. It supports a variety of programming constructs and features:

### Types
- **num**: Numeric values (integers and floating-point)
- **string**: Text values
- **bool**: Boolean values (true/false)
- **void**: For functions that do not return

### Variables and Scope
- Global and local variable declarations
- Block-scoped variables

### Control Flow
- **if/elif/else** conditional statements
- **while** loops for condition-based iteration
- **for** loops for counter-based iteration

### Functions
- User-defined functions with parameters
- Return values
- Recursion support

### Operators
- Standard arithmetic operators: +, -, *, /
- Comparison operators: ==, !=, <, >, <=, >=
- Logical operators: and, or, ! (not)

### Comments
- Single-line comments for code documentation //

## Architecture

Polo uses a multi-stage compilation process:

1. **Scanner**: Converts source code into a stream of tokens
2. **Parser**: Transforms tokens into an Abstract Syntax Tree (AST)
3. **Error Checker**: Validates the AST for semantic correctness
4. **Compiler**: Translates the AST into an intermediate representation (IR)
5. **Linker**: Resolves references within the IR
6. **Virtual Machine**: Executes the linked IR

For details on the language grammar, see the `grammar_current.txt` file included in the repository.

## Example Program

```
// Calculate factorial
num factorial(num n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

void main() {
    num result = factorial(5);
    print result;  // Outputs: 120
}
```
