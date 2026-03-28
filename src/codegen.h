#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

// TAC instruction types
typedef enum {
    TAC_ASSIGN,      // t1 = x
    TAC_BINOP,       // t1 = t2 op t3
    TAC_UNOP,        // t1 = -t2
    TAC_COPY,        // x = t1
    TAC_LABEL,       // L1:
    TAC_GOTO,        // goto L1
    TAC_IF_FALSE,    // if_false t1 goto L1
    TAC_PARAM,       // param t1
    TAC_CALL,        // t1 = call func_name N
    TAC_RETURN,      // return t1
    TAC_WRITE,       // write t1
    TAC_READ,        // read "prompt" -> x
} TACOp;

// TAC instruction
typedef struct TACInstr {
    TACOp op;
    char* result;
    char* arg1;
    char* arg2;
    char* label;
    struct TACInstr* next;
} TACInstr;

// Code generation functions
void codegen_init();
void codegen_program(ASTNode* root);
void codegen_print();
void codegen_free();

// Helper functions
char* gen_temp();
char* gen_label();

#endif /* CODEGEN_H */
