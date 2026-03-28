# Custom Language Compiler

A complete compiler implementation for a custom programming language, built incrementally from lexical analysis through code generation.

## 🎯 Project Overview

This project demonstrates the full compiler development process:
- **Phase 1:** Lexical Analysis (Flex)
- **Phase 2:** Syntax Analysis (Bison)
- **Phase 3:** Abstract Syntax Tree (AST)
- **Phase 4:** Symbol Table Management
- **Phase 5:** Semantic Analysis
- **Phase 6:** Interpreter (Tree-walk)
- **Phase 7:** Intermediate Code Generation (Three-Address Code)

## 🚀 Quick Start

### Prerequisites
- **Flex** (or win_flex on Windows)
- **Bison** (or win_bison on Windows)
- **GCC** (C compiler)
- **Make** (optional, manual build commands provided)

### Build Instructions

```bash
# Generate parser
bison -d parser.y

# Generate lexer
flex flex.l

# Compile all source files
gcc -c -Isrc lex.yy.c parser.tab.c src/ast.c src/symtable.c src/semantic.c src/interpreter.c src/codegen.c src/main.c

# Link executable
gcc -o compiler.exe lex.yy.o parser.tab.o ast.o symtable.o semantic.o interpreter.o codegen.o main.o -lm
```

Or use the Makefile:
```bash
make
```

### Run Tests

```bash
# Run a test program
./compiler.exe tests/test_hello.txt

# View AST
./compiler.exe tests/test_hello.txt --print-ast

# Generate intermediate code
./compiler.exe tests/test_hello.txt --emit-ir
```

## 📚 Language Features

### Entry Point
```
start()[
    // Your code here
]
```

### Variables & I/O
```
x = 10
name = "Alice"
write("Hello, " + name)
```

### Conditionals
```
if(x > 5){
    write("Greater")
}
elif(x == 5){
    write("Equal")
}
else{
    write("Less")
}
```

### Loops
```
// Counted loop with step
loop(i: +1: 0 to 10){
    write(i)
}

// Countdown
loop(j: -1: 10 to 0){
    write(j)
}
```

### Functions
```
add(a, b)[
    return a + b
]

result = add(5, 3)
write(result)
```

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Unary: `-x` (negation)

### Data Types
- Numbers: `42`, `-10`, `3.14`
- Strings: `"Hello"`, `'World'`
- Dynamic typing (no type declarations needed)

## 📁 Project Structure

```
Project/
├── src/              # Source code
│   ├── ast.c/h          # Abstract Syntax Tree
│   ├── symtable.c/h     # Symbol Table
│   ├── semantic.c/h     # Semantic Analyzer
│   ├── interpreter.c/h  # Interpreter
│   ├── codegen.c/h      # Code Generator
│   └── main.c           # Main Driver
├── tests/            # Test programs
├── docs/             # Documentation
├── examples/         # Example inputs
├── flex.l           # Lexical analyzer
├── parser.y         # Parser grammar
├── Makefile         # Build system
└── README.md        # This file
```

## 📖 Documentation

- [PROJECT_OVERVIEW.txt](docs/PROJECT_OVERVIEW.txt) - Complete project documentation
- [BUILD_INSTRUCTIONS.txt](docs/BUILD_INSTRUCTIONS.txt) - Detailed build guide
- [GIT_COMMIT_SEQUENCE.md](GIT_COMMIT_SEQUENCE.md) - Recommended commit order
- [SIMPLE_README.md](docs/SIMPLE_README.md) - Simple lexer tutorial

## 🧪 Testing

12 comprehensive test cases included:
- Basic I/O and variables
- Arithmetic operations
- Conditional statements (if/elif/else)
- Loops (counted, infinite, nested)
- Functions (definition, calls, parameters)
- String concatenation
- Comparison operators
- Error handling (undefined variables, wrong arguments, invalid returns)

Run all tests:
```bash
make test
```

## 🛠️ Development Process

See [GIT_COMMIT_SEQUENCE.md](GIT_COMMIT_SEQUENCE.md) for the recommended sequence of commits that demonstrates proper incremental development.

## 📝 License

Educational project for Compiler Lab coursework.

## 👨‍💻 Author

CSE 3-2 Compiler Lab Project

---

**Note:** This compiler demonstrates all major phases of compilation from lexical analysis through intermediate code generation. It includes both an interpreter for direct execution and a code generator for producing three-address code (TAC) intermediate representation.
