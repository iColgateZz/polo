## General Architecture
- **Scanner:** source code -> tokens
- **Parser:** tokens -> AST
- **Error checking** AST -> success / failure
- **Compiler:** AST -> IR (custom assembly-like code)
- **Linker:** IR -> linked IR
- **VM/Interpreter:** linked IR -> program execution

## Language Features
- var keyword, support for numbers, strings, structs, arrays
- user-defined functions, return
- global/local variables, scope
- for/while loops
- if, if/else
- break/continue
- default mathematical operations and comparison
- comments

## Optional features
- Optimizer IR -> IR (optional)
- interoperability with C? Can call C functions?
