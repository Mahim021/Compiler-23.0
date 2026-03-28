#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Helper to create a new node
static ASTNode* new_node(NodeType type, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->line = line;
    return node;
}

ASTNode* make_number(double val, int line) {
    ASTNode* node = new_node(NODE_NUMBER, line);
    node->num_val = val;
    return node;
}

ASTNode* make_string(char* val, int line) {
    ASTNode* node = new_node(NODE_STRING, line);
    node->str_val = strdup(val);
    return node;
}

ASTNode* make_identifier(char* name, int line) {
    ASTNode* node = new_node(NODE_IDENTIFIER, line);
    node->id_name = strdup(name);
    return node;
}

ASTNode* make_assign(char* name, ASTNode* val, int line) {
    ASTNode* node = new_node(NODE_ASSIGN, line);
    node->assign.name = strdup(name);
    node->assign.value = val;
    return node;
}

ASTNode* make_binop(char* op, ASTNode* left, ASTNode* right, int line) {
    ASTNode* node = new_node(NODE_BINOP, line);
    node->binop.op = strdup(op);
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode* make_unop(char* op, ASTNode* operand, int line) {
    ASTNode* node = new_node(NODE_UNOP, line);
    node->unop.op = strdup(op);
    node->unop.operand = operand;
    return node;
}

ASTNode* make_write(ASTNode* expr, int line) {
    ASTNode* node = new_node(NODE_WRITE, line);
    node->write_node.expr = expr;
    node->write_node.is_concat = 0;
    node->write_node.prefix_str = NULL;
    node->write_node.suffix_id = NULL;
    return node;
}

ASTNode* make_write_concat(char* prefix_str, char* suffix_id, int line) {
    ASTNode* node = new_node(NODE_WRITE, line);
    node->write_node.expr = NULL;
    node->write_node.is_concat = 1;
    node->write_node.prefix_str = strdup(prefix_str);
    node->write_node.suffix_id = strdup(suffix_id);
    return node;
}

ASTNode* make_read(char* prompt, char* var, int line) {
    ASTNode* node = new_node(NODE_READ, line);
    node->read_node.prompt = strdup(prompt);
    node->read_node.var_name = strdup(var);
    return node;
}

ASTNode* make_if(ASTNode* cond, ASTNode* then_body,
                 ASTNode** elif_conds, ASTNode** elif_bodies,
                 int elif_count, ASTNode* else_body, int line) {
    ASTNode* node = new_node(NODE_IF, line);
    node->if_node.cond = cond;
    node->if_node.then_body = then_body;
    node->if_node.elif_conds = elif_conds;
    node->if_node.elif_bodies = elif_bodies;
    node->if_node.elif_count = elif_count;
    node->if_node.else_body = else_body;
    return node;
}

ASTNode* make_loop_counted(char* var, int step,
                            ASTNode* start, ASTNode* end,
                            ASTNode* body, int line) {
    ASTNode* node = new_node(NODE_LOOP_COUNTED, line);
    node->loop_counted.var = strdup(var);
    node->loop_counted.step = step;
    node->loop_counted.start = start;
    node->loop_counted.end = end;
    node->loop_counted.body = body;
    return node;
}

ASTNode* make_loop_infinite(char* var, ASTNode* body,
                             ASTNode* check_start, ASTNode* check_end,
                             int line) {
    ASTNode* node = new_node(NODE_LOOP_INFINITE, line);
    node->loop_infinite.var = strdup(var);
    node->loop_infinite.body = body;
    node->loop_infinite.check_start = check_start;
    node->loop_infinite.check_end = check_end;
    return node;
}

ASTNode* make_func_def(char* name, char** params, int param_count,
                        ASTNode* body, int line) {
    ASTNode* node = new_node(NODE_FUNC_DEF, line);
    node->func_def.name = strdup(name);
    node->func_def.params = params;
    node->func_def.param_count = param_count;
    node->func_def.body = body;
    return node;
}

ASTNode* make_func_call(char* name, ASTNode** args, int arg_count, int line) {
    ASTNode* node = new_node(NODE_FUNC_CALL, line);
    node->func_call.name = strdup(name);
    node->func_call.args = args;
    node->func_call.arg_count = arg_count;
    return node;
}

ASTNode* make_return(ASTNode* val, int line) {
    ASTNode* node = new_node(NODE_RETURN, line);
    node->ret_node.value = val;
    return node;
}

ASTNode* make_stmt_list(ASTNode** stmts, int count, int line) {
    ASTNode* node = new_node(NODE_STMT_LIST, line);
    node->list.stmts = stmts;
    node->list.count = count;
    return node;
}

ASTNode* make_program(ASTNode* stmt_list, int line) {
    ASTNode* node = new_node(NODE_PROGRAM, line);
    node->list.stmts = (ASTNode**)malloc(sizeof(ASTNode*));
    node->list.stmts[0] = stmt_list;
    node->list.count = 1;
    return node;
}

// Print AST with indentation
void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("PROGRAM:\n");
            for (int i = 0; i < node->list.count; i++) {
                print_ast(node->list.stmts[i], indent + 1);
            }
            break;
            
        case NODE_STMT_LIST:
            printf("STMT_LIST:\n");
            for (int i = 0; i < node->list.count; i++) {
                print_ast(node->list.stmts[i], indent + 1);
            }
            break;
            
        case NODE_ASSIGN:
            printf("ASSIGN: %s =\n", node->assign.name);
            print_ast(node->assign.value, indent + 1);
            break;
            
        case NODE_WRITE:
            printf("WRITE:\n");
            if (node->write_node.is_concat) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("CONCAT: \"%s\" + %s\n", 
                       node->write_node.prefix_str, 
                       node->write_node.suffix_id);
            } else {
                print_ast(node->write_node.expr, indent + 1);
            }
            break;
            
        case NODE_READ:
            printf("READ: \"%s\" -> %s\n", 
                   node->read_node.prompt, 
                   node->read_node.var_name);
            break;
            
        case NODE_IF:
            printf("IF:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("CONDITION:\n");
            print_ast(node->if_node.cond, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("THEN:\n");
            print_ast(node->if_node.then_body, indent + 2);
            
            for (int i = 0; i < node->if_node.elif_count; i++) {
                for (int j = 0; j < indent + 1; j++) printf("  ");
                printf("ELIF:\n");
                for (int j = 0; j < indent + 2; j++) printf("  ");
                printf("CONDITION:\n");
                print_ast(node->if_node.elif_conds[i], indent + 3);
                for (int j = 0; j < indent + 2; j++) printf("  ");
                printf("THEN:\n");
                print_ast(node->if_node.elif_bodies[i], indent + 3);
            }
            
            if (node->if_node.else_body) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("ELSE:\n");
                print_ast(node->if_node.else_body, indent + 2);
            }
            break;
            
        case NODE_LOOP_COUNTED:
            printf("LOOP_COUNTED: %s step=%d\n", 
                   node->loop_counted.var, 
                   node->loop_counted.step);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("START:\n");
            print_ast(node->loop_counted.start, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("END:\n");
            print_ast(node->loop_counted.end, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("BODY:\n");
            print_ast(node->loop_counted.body, indent + 2);
            break;
            
        case NODE_LOOP_INFINITE:
            printf("LOOP_INFINITE: %s\n", node->loop_infinite.var);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("BODY:\n");
            print_ast(node->loop_infinite.body, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("CHECK: start to end\n");
            print_ast(node->loop_infinite.check_start, indent + 2);
            print_ast(node->loop_infinite.check_end, indent + 2);
            break;
            
        case NODE_FUNC_DEF:
            printf("FUNC_DEF: %s(", node->func_def.name);
            for (int i = 0; i < node->func_def.param_count; i++) {
                printf("%s", node->func_def.params[i]);
                if (i < node->func_def.param_count - 1) printf(", ");
            }
            printf(")\n");
            print_ast(node->func_def.body, indent + 1);
            break;
            
        case NODE_FUNC_CALL:
            printf("FUNC_CALL: %s(", node->func_call.name);
            printf(")\n");
            for (int i = 0; i < node->func_call.arg_count; i++) {
                print_ast(node->func_call.args[i], indent + 1);
            }
            break;
            
        case NODE_RETURN:
            printf("RETURN:\n");
            if (node->ret_node.value) {
                print_ast(node->ret_node.value, indent + 1);
            }
            break;
            
        case NODE_BINOP:
            printf("BINOP: %s\n", node->binop.op);
            print_ast(node->binop.left, indent + 1);
            print_ast(node->binop.right, indent + 1);
            break;
            
        case NODE_UNOP:
            printf("UNOP: %s\n", node->unop.op);
            print_ast(node->unop.operand, indent + 1);
            break;
            
        case NODE_NUMBER:
            printf("NUMBER: %.10g\n", node->num_val);
            break;
            
        case NODE_STRING:
            printf("STRING: \"%s\"\n", node->str_val);
            break;
            
        case NODE_IDENTIFIER:
            printf("IDENTIFIER: %s\n", node->id_name);
            break;
            
        default:
            printf("UNKNOWN NODE TYPE\n");
            break;
    }
}

// Free AST recursively
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_STMT_LIST:
            for (int i = 0; i < node->list.count; i++) {
                free_ast(node->list.stmts[i]);
            }
            free(node->list.stmts);
            break;
            
        case NODE_ASSIGN:
            free(node->assign.name);
            free_ast(node->assign.value);
            break;
            
        case NODE_WRITE:
            if (node->write_node.is_concat) {
                free(node->write_node.prefix_str);
                free(node->write_node.suffix_id);
            } else {
                free_ast(node->write_node.expr);
            }
            break;
            
        case NODE_READ:
            free(node->read_node.prompt);
            free(node->read_node.var_name);
            break;
            
        case NODE_IF:
            free_ast(node->if_node.cond);
            free_ast(node->if_node.then_body);
            for (int i = 0; i < node->if_node.elif_count; i++) {
                free_ast(node->if_node.elif_conds[i]);
                free_ast(node->if_node.elif_bodies[i]);
            }
            if (node->if_node.elif_count > 0) {
                free(node->if_node.elif_conds);
                free(node->if_node.elif_bodies);
            }
            free_ast(node->if_node.else_body);
            break;
            
        case NODE_LOOP_COUNTED:
            free(node->loop_counted.var);
            free_ast(node->loop_counted.start);
            free_ast(node->loop_counted.end);
            free_ast(node->loop_counted.body);
            break;
            
        case NODE_LOOP_INFINITE:
            free(node->loop_infinite.var);
            free_ast(node->loop_infinite.body);
            free_ast(node->loop_infinite.check_start);
            free_ast(node->loop_infinite.check_end);
            break;
            
        case NODE_FUNC_DEF:
            free(node->func_def.name);
            for (int i = 0; i < node->func_def.param_count; i++) {
                free(node->func_def.params[i]);
            }
            if (node->func_def.param_count > 0) {
                free(node->func_def.params);
            }
            free_ast(node->func_def.body);
            break;
            
        case NODE_FUNC_CALL:
            free(node->func_call.name);
            for (int i = 0; i < node->func_call.arg_count; i++) {
                free_ast(node->func_call.args[i]);
            }
            if (node->func_call.arg_count > 0) {
                free(node->func_call.args);
            }
            break;
            
        case NODE_RETURN:
            free_ast(node->ret_node.value);
            break;
            
        case NODE_BINOP:
            free(node->binop.op);
            free_ast(node->binop.left);
            free_ast(node->binop.right);
            break;
            
        case NODE_UNOP:
            free(node->unop.op);
            free_ast(node->unop.operand);
            break;
            
        case NODE_STRING:
            free(node->str_val);
            break;
            
        case NODE_IDENTIFIER:
            free(node->id_name);
            break;
            
        case NODE_NUMBER:
            // No dynamic memory for numbers
            break;
            
        default:
            break;
    }
    
    free(node);
}
