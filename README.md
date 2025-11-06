## General Architecture
- **Scanner:** source code -> tokens
- **Parser:** tokens -> AST
- **Compiler:** AST -> IR (custom assembly-like code)
- **Linker:** IR -> linked IR
- **VM/Interpreter:** linked IR -> program execution

## Language Features
- data types: int, float, string, struct, pointers
- user-defined functions
- for/while loops
- if, if/else
- break/continue
- default mathematical operations and comparison

## Optional features
- Optimizer IR -> IR (optional)
- interoperability with C? Can call C functions?
