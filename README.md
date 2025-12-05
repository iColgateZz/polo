<div align="center">
  <h1>Polo Programming Language</h1>
  <p><em>A modern, statically-typed language with clean syntax and efficient execution</em></p>
  
  ![License](https://img.shields.io/badge/license-MIT-blue)
</div>

## Introduction

Polo is a general-purpose programming language designed with simplicity and expressiveness in mind. It offers a clean syntax with static typing, allowing developers to write concise yet readable code. Polo is built with a complete compiler pipeline from lexical analysis to bytecode execution.

## Quick Start

```bash
# Clone the repository
git clone https://github.com/iColgateZz/polo.git
cd polo

# Build the compiler
make

# Run the hello world example
./polo examples/hello.polo
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

More examples can be found in the `examples` folder.
For details on the language grammar, see the `grammar_current.txt` file included in the repository.

## Architecture

Polo uses a multi-stage compilation process:

1. **Scanner**: Converts source code into a stream of tokens
2. **Parser**: Transforms tokens into an Abstract Syntax Tree (AST)
3. **Error Checker**: Validates the AST for semantic correctness
4. **Compiler**: Translates the AST into an intermediate representation (IR)
5. **Linker**: Resolves references within the IR
6. **Virtual Machine**: Executes the linked IR

## Technical Highlights

- **Custom Stack-Based VM**: Implemented a virtual machine with optimized instruction set
- **Robust Type System**: Built a complete static type checking system
- **Error Recovery**: Sophisticated error handling that allows the compiler to continue after errors
- **Multi-stage Compilation**: From source code to bytecode with intermediate representations

## Demonstrated Skills

- **Languages**: C for compiler and VM implementation
- **Algorithms**: Recursive descent parsing, symbol resolution, dfs linking
- **System Design**: Multi-stage compilation pipeline with clear interfaces between components

## Lessons Learned & Future Development

### What I Learned

Developing Polo has been an educational journey that provided deep insights into several areas:

1. **Compiler Design**: I gained hands-on experience with the entire compilation pipeline from lexical analysis to code generation, understanding how programming languages are implemented at a fundamental level.

2. **Language Design**: Balancing simplicity with expressiveness is challenging. Every design decision impacts both the language's usability and implementation complexity.

3. **Virtual Machine Implementation**: Building a bytecode VM taught me about runtime environments, instruction sets, and stack-based execution models.

4. **Error Handling**: Creating meaningful error messages and recovery mechanisms is as important as the language features themselves.

### Future Enhancements

There are several exciting features that could be added to Polo:

1. **Composite Data Types**:
   - Arrays and array indexing
   - Structs for custom data types

2. **Advanced Function Features**:
   - First-class functions with function type syntax (`fn (num, bool) -> string`)

3. **Module System**:
   - Import statements for code organization and reusability

4. **Standard Library**:
    - File IO
    - String manipulation
    - Advanced mathematical operations

5. **Control Flow**:
   - `break` and `continue` statements for loop control

These additions would transform Polo from a simple language into a more powerful general-purpose language while maintaining its clean syntax and approachable nature.
