#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "symtable.h"

int semantic_error_count = 0;
int semantic_warning_count = 0;

// Track declared variables per scope (simplified - just track locally)
typedef struct VarSet {
    char** names;
    int count;
    int capacity;
} VarSet;

static VarSet* current_vars = NULL;
static int in_function = 0;

// Function registry for hoisting
typedef struct FuncInfo {
    char* name;
    int param_count;
    int line;
} FuncInfo;

static FuncInfo* functions = NULL;
static int func_count = 0;
static int func_capacity = 0;

static void varset_init() {
    current_vars = (VarSet*)malloc(sizeof(VarSet));
    current_vars->count = 0;
    current_vars->capacity = 16;
    current_vars->names = (char**)malloc(sizeof(char*) * current_vars->capacity);
}

static void varset_free() {
    if (!current_vars) return;
    for (int i = 0; i < current_vars->count; i++) {
        free(current_vars->names[i]);
    }
    free(current_vars->names);
    free(current_vars);
    current_vars = NULL;
}

static void varset_add(char* name) {
    if (!current_vars) varset_init();
    
    // Check if already exists
    for (int i = 0; i < current_vars->count; i++) {
        if (strcmp(current_vars->names[i], name) == 0) {
            return;
        }
    }
    
    if (current_vars->count >= current_vars->capacity) {
        current_vars->capacity *= 2;
        current_vars->names = (char**)realloc(current_vars->names, 
                                               sizeof(char*) * current_vars->capacity);
    }
    current_vars->names[current_vars->count++] = strdup(name);
}

static int varset_contains(char* name) {
    if (!current_vars) return 0;
    for (int i = 0; i < current_vars->count; i++) {
        if (strcmp(current_vars->names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void register_function(char* name, int param_count, int line) {
    if (func_count >= func_capacity) {
        func_capacity = func_capacity == 0 ? 16 : func_capacity * 2;
        functions = (FuncInfo*)realloc(functions, sizeof(FuncInfo) * func_capacity);
    }
    functions[func_count].name = strdup(name);
    functions[func_count].param_count = param_count;
    functions[func_count].line = line;
    func_count++;
}

static FuncInfo* find_function(char* name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    return NULL;
}

// Forward declarations
static void check_stmt(ASTNode* node);
static void check_expr(ASTNode* node);

// First pass: collect all function definitions
static void collect_functions(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_STMT_LIST:
            for (int i = 0; i < node->list.count; i++) {
                collect_functions(node->list.stmts[i]);
            }
            break;
            
        case NODE_FUNC_DEF:
            register_function(node->func_def.name, node->func_def.param_count, node->line);
            collect_functions(node->func_def.body);
            break;
            
        case NODE_IF:
            collect_functions(node->if_node.then_body);
            for (int i = 0; i < node->if_node.elif_count; i++) {
                collect_functions(node->if_node.elif_bodies[i]);
            }
            collect_functions(node->if_node.else_body);
            break;
            
        case NODE_LOOP_COUNTED:
            collect_functions(node->loop_counted.body);
            break;
            
        case NODE_LOOP_INFINITE:
            collect_functions(node->loop_infinite.body);
            break;
            
        default:
            break;
    }
}

static void check_expr(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_IDENTIFIER:
            if (!varset_contains(node->id_name)) {
                fprintf(stderr, "Semantic Error [line %d]: Undefined variable '%s'\n",
                        node->line, node->id_name);
                semantic_error_count++;
            }
            break;
            
        case NODE_BINOP:
            check_expr(node->binop.left);
            check_expr(node->binop.right);
            break;
            
        case NODE_UNOP:
            check_expr(node->unop.operand);
            break;
            
        case NODE_FUNC_CALL: {
            FuncInfo* func = find_function(node->func_call.name);
            
            // Check for built-in functions
            if (strcmp(node->func_call.name, "write") == 0 ||
                strcmp(node->func_call.name, "read") == 0) {
                // Built-ins are always valid (already checked in parser)
                for (int i = 0; i < node->func_call.arg_count; i++) {
                    check_expr(node->func_call.args[i]);
                }
                break;
            }
            
            if (!func) {
                fprintf(stderr, "Semantic Error [line %d]: Function '%s' is not defined\n",
                        node->line, node->func_call.name);
                semantic_error_count++;
            } else if (func->param_count != node->func_call.arg_count) {
                fprintf(stderr, "Semantic Error [line %d]: Argument count mismatch calling '%s': "
                        "expected %d got %d\n",
                        node->line, node->func_call.name, 
                        func->param_count, node->func_call.arg_count);
                semantic_error_count++;
            }
            
            for (int i = 0; i < node->func_call.arg_count; i++) {
                check_expr(node->func_call.args[i]);
            }
            break;
        }
            
        case NODE_NUMBER:
        case NODE_STRING:
            // Literals are always valid
            break;
            
        default:
            break;
    }
}

static void check_stmt(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_STMT_LIST:
            for (int i = 0; i < node->list.count; i++) {
                check_stmt(node->list.stmts[i]);
            }
            break;
            
        case NODE_ASSIGN:
            check_expr(node->assign.value);
            varset_add(node->assign.name);  // Declare variable
            break;
            
        case NODE_WRITE:
            if (node->write_node.is_concat) {
                if (!varset_contains(node->write_node.suffix_id)) {
                    fprintf(stderr, "Semantic Error [line %d]: Undefined variable '%s'\n",
                            node->line, node->write_node.suffix_id);
                    semantic_error_count++;
                }
            } else {
                check_expr(node->write_node.expr);
            }
            break;
            
        case NODE_READ:
            // read() declares the variable
            varset_add(node->read_node.var_name);
            break;
            
        case NODE_IF:
            check_expr(node->if_node.cond);
            check_stmt(node->if_node.then_body);
            
            for (int i = 0; i < node->if_node.elif_count; i++) {
                check_expr(node->if_node.elif_conds[i]);
                check_stmt(node->if_node.elif_bodies[i]);
            }
            
            check_stmt(node->if_node.else_body);
            break;
            
        case NODE_LOOP_COUNTED:
            // Loop variable is auto-declared
            if (varset_contains(node->loop_counted.var)) {
                fprintf(stderr, "Semantic Warning [line %d]: Loop variable '%s' shadows outer variable\n",
                        node->line, node->loop_counted.var);
                semantic_warning_count++;
            }
            varset_add(node->loop_counted.var);
            
            check_expr(node->loop_counted.start);
            check_expr(node->loop_counted.end);
            check_stmt(node->loop_counted.body);
            break;
            
        case NODE_LOOP_INFINITE:
            if (varset_contains(node->loop_infinite.var)) {
                fprintf(stderr, "Semantic Warning [line %d]: Loop variable '%s' shadows outer variable\n",
                        node->line, node->loop_infinite.var);
                semantic_warning_count++;
            }
            varset_add(node->loop_infinite.var);
            
            check_expr(node->loop_infinite.check_start);
            check_expr(node->loop_infinite.check_end);
            check_stmt(node->loop_infinite.body);
            break;
            
        case NODE_FUNC_DEF: {
            int was_in_function = in_function;
            in_function = 1;
            
            // Parameters are declared in function scope
            for (int i = 0; i < node->func_def.param_count; i++) {
                varset_add(node->func_def.params[i]);
            }
            
            check_stmt(node->func_def.body);
            
            in_function = was_in_function;
            break;
        }
            
        case NODE_FUNC_CALL:
            check_expr(node);  // Reuse expr checking
            break;
            
        case NODE_RETURN:
            if (!in_function) {
                fprintf(stderr, "Semantic Error [line %d]: return statement outside function\n",
                        node->line);
                semantic_error_count++;
            }
            check_expr(node->ret_node.value);
            break;
            
        default:
            break;
    }
}

int semantic_check(ASTNode* root) {
    if (!root) return 0;
    
    semantic_error_count = 0;
    semantic_warning_count = 0;
    
    // Initialize variable tracking
    varset_init();
    
    // First pass: collect all functions (for hoisting)
    collect_functions(root);
    
    // Second pass: check semantics
    check_stmt(root);
    
    // Cleanup
    varset_free();
    
    return semantic_error_count;
}

void semantic_report() {
    if (semantic_error_count > 0) {
        fprintf(stderr, "\nSemantic analysis found %d error(s)", semantic_error_count);
        if (semantic_warning_count > 0) {
            fprintf(stderr, " and %d warning(s)", semantic_warning_count);
        }
        fprintf(stderr, ".\n");
    } else if (semantic_warning_count > 0) {
        fprintf(stderr, "\nSemantic analysis found %d warning(s).\n", semantic_warning_count);
    }
    
    // Free function registry
    for (int i = 0; i < func_count; i++) {
        free(functions[i].name);
    }
    free(functions);
    functions = NULL;
    func_count = 0;
    func_capacity = 0;
}
