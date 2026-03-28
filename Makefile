# Compiler settings
CC = gcc
CFLAGS = -Wall -Wno-unused-function -g -std=c99

# Windows-specific settings (comment out on Linux/Mac)
ifeq ($(OS),Windows_NT)
    FLEX = win_flex
    BISON = win_bison
    EXECUTABLE = compiler.exe
    RM = del /Q
    RM_DIR = rmdir /S /Q
else
    FLEX = flex
    BISON = bison
    EXECUTABLE = compiler
    RM = rm -f
    RM_DIR = rm -rf
endif

# Source files
LEX_SRC = flex.l
YACC_SRC = parser.y
SRC_DIR = src
C_SOURCES = $(SRC_DIR)/ast.c $(SRC_DIR)/symtable.c $(SRC_DIR)/semantic.c $(SRC_DIR)/interpreter.c $(SRC_DIR)/codegen.c $(SRC_DIR)/main.c

# Generated files
LEX_GEN = lex.yy.c
YACC_GEN = parser.tab.c parser.tab.h

# Object files
OBJECTS = lex.yy.o parser.tab.o ast.o symtable.o semantic.o interpreter.o codegen.o main.o

# Default target
all: $(EXECUTABLE)

# Generate lexer
$(LEX_GEN): $(LEX_SRC) $(YACC_GEN)
	$(FLEX) $(LEX_SRC)

# Generate parser
$(YACC_GEN): $(YACC_SRC)
	$(BISON) -d $(YACC_SRC)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# Link executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) -lm

# Dependencies
lex.yy.o: lex.yy.c parser.tab.h
parser.tab.o: parser.tab.c $(SRC_DIR)/ast.h
ast.o: $(SRC_DIR)/ast.c $(SRC_DIR)/ast.h
symtable.o: $(SRC_DIR)/symtable.c $(SRC_DIR)/symtable.h $(SRC_DIR)/ast.h
semantic.o: $(SRC_DIR)/semantic.c $(SRC_DIR)/semantic.h $(SRC_DIR)/ast.h $(SRC_DIR)/symtable.h
interpreter.o: $(SRC_DIR)/interpreter.c $(SRC_DIR)/interpreter.h $(SRC_DIR)/ast.h $(SRC_DIR)/symtable.h
codegen.o: $(SRC_DIR)/codegen.c $(SRC_DIR)/codegen.h $(SRC_DIR)/ast.h
main.o: $(SRC_DIR)/main.c $(SRC_DIR)/ast.h $(SRC_DIR)/semantic.h $(SRC_DIR)/interpreter.h $(SRC_DIR)/codegen.h

# Run compiler on input.txt
run: $(EXECUTABLE)
	./$(EXECUTABLE) examples/input.txt

# Run with AST printing
run-ast: $(EXECUTABLE)
	./$(EXECUTABLE) examples/input.txt --print-ast

# Run with IR generation
run-ir: $(EXECUTABLE)
	./$(EXECUTABLE) examples/input.txt --emit-ir

# Run with all options
run-all: $(EXECUTABLE)
	./$(EXECUTABLE) examples/input.txt --print-ast --emit-ir

# Test parse only (no execution)
test-parse: $(EXECUTABLE)
	./$(EXECUTABLE) examples/input.txt --no-run

# Run test suite
test: $(EXECUTABLE)
	@echo "Running test suite..."
	./$(EXECUTABLE) tests/test_hello.txt
	./$(EXECUTABLE) tests/test_math.txt
	./$(EXECUTABLE) tests/test_conditionals.txt
	./$(EXECUTABLE) tests/test_loops.txt
	./$(EXECUTABLE) tests/test_functions.txt

# Clean generated files
clean:
ifeq ($(OS),Windows_NT)
	$(RM) $(EXECUTABLE) lex.yy.c parser.tab.c parser.tab.h *.o 2>nul
else
	$(RM) $(EXECUTABLE) lex.yy.c parser.tab.c parser.tab.h *.o
endif

# Clean everything including backups
distclean: clean
ifeq ($(OS),Windows_NT)
	$(RM) *~ *.bak 2>nul
else
	$(RM) *~ *.bak
endif

# Help
help:
	@echo "Available targets:"
	@echo "  make           - Build the compiler"
	@echo "  make run       - Build and run on input.txt"
	@echo "  make run-ast   - Run with AST printing"
	@echo "  make run-ir    - Run with IR generation"
	@echo "  make run-all   - Run with all debug options"
	@echo "  make test-parse- Parse only, don't execute"
	@echo "  make clean     - Remove generated files"
	@echo "  make distclean - Remove all temporary files"
	@echo "  make help      - Show this help message"

.PHONY: all run run-ast run-ir run-all test-parse clean distclean help
