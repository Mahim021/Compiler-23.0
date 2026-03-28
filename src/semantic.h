#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

// Semantic analysis functions
int semantic_check(ASTNode* root);
void semantic_report();

// Error tracking
extern int semantic_error_count;
extern int semantic_warning_count;

#endif /* SEMANTIC_H */
