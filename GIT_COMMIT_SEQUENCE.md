# Git Commit Sequence Guide

This document outlines the **recommended sequence** for pushing files to GitHub to demonstrate the proper development workflow of this compiler project.

---

## 📋 **Purpose**

This sequence shows the **natural progression** of compiler development:
1. Documentation and planning first
2. Lexical analysis (Phase 1)
3. Parser and semantic analysis (Phase 2-4)
4. Execution and code generation (Phase 5-7)
5. Testing and validation

Following this order demonstrates a **systematic, incremental approach** to building a compiler from scratch.

---

## 🚀 **Commit Sequence**

### **COMMIT 1: Project Initialization & Documentation**
**Message:** `Initial commit: Project documentation and overview`

**Files to push:**
```
docs/PROJECT_OVERVIEW.txt
docs/SIMPLE_README.md
README.md (create if needed)
.gitignore (create if needed)
```

**Why first?** 
- Establishes project context
- Shows planning before coding
- Documents the custom language specification

---

### **COMMIT 2: Phase 1 - Simple Lexer (Learning)**
**Message:** `Phase 1a: Simple lexer example for learning Flex basics`

**Files to push:**
```
examples/simple_lexer.l
examples/input.txt
examples/output.txt
```

**Why?**
- Shows initial exploration and learning
- Demonstrates understanding of basic Flex syntax
- Provides a simple reference implementation

---

### **COMMIT 3: Phase 1 - Complete Lexical Analyzer**
**Message:** `Phase 1b: Complete lexical analyzer for custom language`

**Files to push:**
```
flex.l
```

**Why?**
- Core lexical analysis implementation
- Handles all tokens: keywords, operators, delimiters, strings, numbers, comments
- Foundation for parser integration

---

### **COMMIT 4: Phase 2 - Parser Specification**
**Message:** `Phase 2: Bison parser grammar for custom language`

**Files to push:**
```
parser.y
```

**Why?**
- Defines complete grammar
- Shows syntax structure: start()[], if/elif/else, loop(), functions
- Critical for syntax-directed translation

---

### **COMMIT 5: Phase 3 - Abstract Syntax Tree**
**Message:** `Phase 3: AST data structures and builders`

**Files to push:**
```
src/ast.h
src/ast.c
```

**Why?**
- Defines tree representation of programs
- 22 node types for all language constructs
- Enables tree-walk interpretation and code generation

---

### **COMMIT 6: Phase 4 - Symbol Table**
**Message:** `Phase 4: Symbol table with dynamic typing support`

**Files to push:**
```
src/symtable.h
src/symtable.c
```

**Why?**
- Hash table-based scope management
- Supports dynamic typing (numbers, strings, booleans, functions)
- Essential for semantic analysis and execution

---

### **COMMIT 7: Phase 5 - Semantic Analysis**
**Message:** `Phase 5: Semantic analyzer with type checking and validation`

**Files to push:**
```
src/semantic.h
src/semantic.c
```

**Why?**
- Function hoisting and validation
- Variable usage checking
- Error detection before execution

---

### **COMMIT 8: Phase 6 - Interpreter**
**Message:** `Phase 6: Tree-walk interpreter implementation`

**Files to push:**
```
src/interpreter.h
src/interpreter.c
```

**Why?**
- Executes AST directly
- Implements all operators, loops, conditionals, functions
- Built-in functions: write(), read()

---

### **COMMIT 9: Phase 7 - Code Generation**
**Message:** `Phase 7: Three-address code (TAC) generator`

**Files to push:**
```
src/codegen.h
src/codegen.c
```

**Why?**
- Generates intermediate representation (IR)
- TAC instructions: ASSIGN, BINOP, LABEL, GOTO, CALL, RETURN
- Foundation for future optimizations or target code generation

---

### **COMMIT 10: Main Driver & Build System**
**Message:** `Build system: Main driver and Makefile`

**Files to push:**
```
src/main.c
Makefile
```

**Why?**
- Ties all phases together
- CLI options: --print-ast, --emit-ir, --no-run
- Cross-platform build automation

---

### **COMMIT 11: Build Instructions**
**Message:** `Documentation: Complete build and testing instructions`

**Files to push:**
```
docs/BUILD_INSTRUCTIONS.txt
```

**Why?**
- Guides users through installation and compilation
- Provides troubleshooting tips
- Documents all CLI options

---

### **COMMIT 12: Test Suite**
**Message:** `Testing: Comprehensive test cases for all language features`

**Files to push:**
```
tests/test_hello.txt
tests/test_math.txt
tests/test_conditionals.txt
tests/test_loops.txt
tests/test_functions.txt
tests/test_strings.txt
tests/test_nested.txt
tests/test_comparisons.txt
tests/test_complex.txt
tests/test_error_undefined.txt
tests/test_error_args.txt
tests/test_error_return.txt
```

**Why?**
- Validates all implemented features
- Demonstrates language capabilities
- Tests error handling

---

### **COMMIT 13: Generated Files & Build Artifacts** (Optional)
**Message:** `Build artifacts: Generated lexer and parser files`

**Files to push (if desired):**
```
lex.yy.c
parser.tab.c
parser.tab.h
```

**⚠️ Note:** These are **generated files**. It's common practice to **NOT** push them to Git (add to `.gitignore`). However, pushing them can help others who don't have Flex/Bison installed.

---

### **COMMIT 14: Compiled Executables** (Usually NOT pushed)
**Message:** `Executable binaries`

**Files:**
```
compiler.exe (or a.out on Linux)
lexer.exe
*.o (object files)
```

**⚠️ Important:** 
- Executables are **platform-specific** and **large**
- Standard practice: Add to `.gitignore`
- Only push if specifically needed for distribution

---

## 📝 **Recommended .gitignore**

Create a `.gitignore` file to exclude generated and compiled files:

```gitignore
# Generated by Flex
lex.yy.c

# Generated by Bison
parser.tab.c
parser.tab.h
parser.output

# Compiled object files
*.o

# Executables
*.exe
*.out
a.out
compiler
compiler.exe
lexer.exe

# Editor/IDE files (already likely ignored)
.vscode/
.vs/
*.swp
*~

# OS files
.DS_Store
Thumbs.db
```

---

## 🔄 **How to Follow This Sequence**

### **Option 1: Push commits incrementally**
```bash
# Commit 1
git add docs/PROJECT_OVERVIEW.txt docs/SIMPLE_README.md README.md
git commit -m "Initial commit: Project documentation and overview"
git push

# Commit 2
git add examples/simple_lexer.l examples/input.txt examples/output.txt
git commit -m "Phase 1a: Simple lexer example for learning Flex basics"
git push

# Continue for commits 3-12...
```

### **Option 2: Push all at once but preserve commit history**
```bash
# Stage and commit each phase separately
git add docs/PROJECT_OVERVIEW.txt docs/SIMPLE_README.md
git commit -m "Initial commit: Project documentation and overview"

git add examples/simple_lexer.l examples/input.txt examples/output.txt
git commit -m "Phase 1a: Simple lexer example for learning Flex basics"

git add flex.l
git commit -m "Phase 1b: Complete lexical analyzer for custom language"

# ... continue for all phases ...

# Then push all commits at once
git push origin main
```

---

## ✅ **Verification Checklist**

After pushing each commit, verify:
- [ ] Files are in correct folders (src/, tests/, docs/, examples/)
- [ ] Commit message clearly describes the phase
- [ ] Each commit represents a **logical unit of work**
- [ ] Progression follows compiler development stages
- [ ] No unnecessary generated files pushed (check .gitignore)

---

## 💡 **Tips for Clean Git History**

1. **Atomic Commits:** Each commit should represent one complete feature or phase
2. **Clear Messages:** Use descriptive commit messages that explain WHAT and WHY
3. **Test Before Committing:** Ensure code compiles (if applicable) before committing
4. **Document First:** Push documentation before implementation files
5. **Logical Order:** Show progression from simple to complex

---

## 📊 **Project Structure After Organization**

```
Project/
├── .git/
├── .gitignore
├── README.md
├── GIT_COMMIT_SEQUENCE.md (this file)
├── Makefile
├── flex.l
├── parser.y
├── src/
│   ├── ast.h
│   ├── ast.c
│   ├── symtable.h
│   ├── symtable.c
│   ├── semantic.h
│   ├── semantic.c
│   ├── interpreter.h
│   ├── interpreter.c
│   ├── codegen.h
│   ├── codegen.c
│   └── main.c
├── tests/
│   ├── test_hello.txt
│   ├── test_math.txt
│   ├── test_conditionals.txt
│   ├── test_loops.txt
│   ├── test_functions.txt
│   ├── test_strings.txt
│   ├── test_nested.txt
│   ├── test_comparisons.txt
│   ├── test_complex.txt
│   ├── test_error_undefined.txt
│   ├── test_error_args.txt
│   └── test_error_return.txt
├── docs/
│   ├── PROJECT_OVERVIEW.txt
│   ├── BUILD_INSTRUCTIONS.txt
│   └── SIMPLE_README.md
└── examples/
    ├── simple_lexer.l
    ├── input.txt
    └── output.txt
```

---

## 🎯 **Summary**

This sequence demonstrates:
1. **Thoughtful planning** (documentation first)
2. **Incremental development** (phase by phase)
3. **Proper testing** (test suite at the end)
4. **Professional workflow** (organized structure, clear commits)

Following this order shows reviewers that you built the compiler **systematically** and **understood each phase** before moving to the next. It's **not just a random dump of files** — it's a **documented development journey**.

**Good luck with your compiler project! 🚀**
