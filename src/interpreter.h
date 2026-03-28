#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "symtable.h"

// Interpreter functions
void interpret(ASTNode* root);
Value eval_expr(ASTNode* node);
void exec_stmt(ASTNode* node);

// Return value handling
extern int return_flag;
extern Value return_value;

#endif /* INTERPRETER_H */
