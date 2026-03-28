#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantic.h"
#include "interpreter.h"
#include "codegen.h"

// External declarations from parser
extern int yyparse();
extern FILE* yyin;
extern ASTNode* ast_root;
extern int parse_error_count;
extern int yylineno;

// Command line flags
static int flag_print_ast = 0;
static int flag_emit_ir = 0;
static int flag_no_run = 0;

static void print_usage(const char* prog_name) {
    printf("Usage: %s <source_file> [options]\n", prog_name);
    printf("Options:\n");
    printf("  --print-ast    Print the abstract syntax tree\n");
    printf("  --emit-ir      Generate and print three-address code (TAC)\n");
    printf("  --no-run       Don't execute the program (only parse and check)\n");
    printf("\nExample:\n");
    printf("  %s input.txt\n", prog_name);
    printf("  %s input.txt --print-ast\n", prog_name);
    printf("  %s input.txt --emit-ir --no-run\n", prog_name);
}

static void parse_arguments(int argc, char** argv, char** source_file) {
    if (argc < 2) {
        print_usage(argv[0]);
        exit(1);
    }
    
    *source_file = argv[1];
    
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            flag_print_ast = 1;
        } else if (strcmp(argv[i], "--emit-ir") == 0) {
            flag_emit_ir = 1;
        } else if (strcmp(argv[i], "--no-run") == 0) {
            flag_no_run = 1;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            exit(1);
        }
    }
}

int main(int argc, char** argv) {
    char* source_file = NULL;
    
    // Parse command line arguments
    parse_arguments(argc, argv, &source_file);
    
    // Open source file
    FILE* file = fopen(source_file, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", source_file);
        return 1;
    }
    
    yyin = file;
    
    // Phase 1: Parsing
    printf("=== Parsing %s ===\n", source_file);
    int parse_result = yyparse();
    
    fclose(file);
    
    if (parse_result != 0 || parse_error_count > 0) {
        fprintf(stderr, "\nParsing failed with %d error(s).\n", parse_error_count);
        if (ast_root) free_ast(ast_root);
        return 1;
    }
    
    if (!ast_root) {
        fprintf(stderr, "\nError: No program parsed.\n");
        return 1;
    }
    
    printf("Parsing successful!\n\n");
    
    // Phase 2: Semantic Analysis
    printf("=== Semantic Analysis ===\n");
    int semantic_errors = semantic_check(ast_root);
    semantic_report();
    
    if (semantic_errors > 0) {
        fprintf(stderr, "\nSemantic analysis failed. Aborting.\n");
        free_ast(ast_root);
        return 1;
    }
    
    printf("Semantic analysis passed!\n\n");
    
    // Optional: Print AST
    if (flag_print_ast) {
        printf("=== Abstract Syntax Tree ===\n");
        print_ast(ast_root, 0);
        printf("\n");
    }
    
    // Optional: Generate IR
    if (flag_emit_ir) {
        printf("=== Intermediate Code Generation ===\n");
        codegen_init();
        codegen_program(ast_root);
        codegen_print();
        codegen_free();
    }
    
    // Phase 3: Interpretation
    if (!flag_no_run) {
        printf("=== Program Execution ===\n");
        interpret(ast_root);
        printf("\n=== Execution Complete ===\n");
    }
    
    // Cleanup
    free_ast(ast_root);
    
    return 0;
}
