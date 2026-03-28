#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static TACInstr* tac_head = NULL;
static TACInstr* tac_tail = NULL;
static int temp_count = 0;
static int label_count = 0;

// Generate new temporary variable
char* gen_temp() {
    char* temp = (char*)malloc(16);
    snprintf(temp, 16, "t%d", temp_count++);
    return temp;
}

// Generate new label
char* gen_label() {
    char* label = (char*)malloc(16);
    snprintf(label, 16, "L%d", label_count++);
    return label;
}

// Create new TAC instruction
static TACInstr* new_instr(TACOp op) {
    TACInstr* instr = (TACInstr*)malloc(sizeof(TACInstr));
    memset(instr, 0, sizeof(TACInstr));
    instr->op = op;
    instr->next = NULL;
    return instr;
}

// Emit TAC instruction
static void emit(TACOp op, char* result, char* arg1, char* arg2) {
    TACInstr* instr = new_instr(op);
    instr->result = result ? strdup(result) : NULL;
    instr->arg1 = arg1 ? strdup(arg1) : NULL;
    instr->arg2 = arg2 ? strdup(arg2) : NULL;
    
    if (!tac_head) {
        tac_head = tac_tail = instr;
    } else {
        tac_tail->next = instr;
        tac_tail = instr;
    }
}

// Emit label
static void emit_label(char* label) {
    TACInstr* instr = new_instr(TAC_LABEL);
    instr->label = strdup(label);
    
    if (!tac_head) {
        tac_head = tac_tail = instr;
    } else {
        tac_tail->next = instr;
        tac_tail = instr;
    }
}

// Forward declarations
static char* gen_expr(ASTNode* node);
static void gen_stmt(ASTNode* node);

// Generate TAC for expression
static char* gen_expr(ASTNode* node) {
    if (!node) return NULL;
    
    switch (node->type) {
        case NODE_NUMBER: {
            char* temp = gen_temp();
            char num_str[64];
            if (node->num_val == (int)node->num_val) {
                snprintf(num_str, sizeof(num_str), "%d", (int)node->num_val);
            } else {
                snprintf(num_str, sizeof(num_str), "%.10g", node->num_val);
            }
            emit(TAC_ASSIGN, temp, num_str, NULL);
            return temp;
        }
            
        case NODE_STRING: {
            char* temp = gen_temp();
            char str[1024];
            snprintf(str, sizeof(str), "\"%s\"", node->str_val);
            emit(TAC_ASSIGN, temp, str, NULL);
            return temp;
        }
            
        case NODE_IDENTIFIER: {
            char* temp = gen_temp();
            emit(TAC_ASSIGN, temp, node->id_name, NULL);
            return temp;
        }
            
        case NODE_BINOP: {
            char* left = gen_expr(node->binop.left);
            char* right = gen_expr(node->binop.right);
            char* temp = gen_temp();
            emit(TAC_BINOP, temp, left, right);
            free(left);
            free(right);
            return temp;
        }
            
        case NODE_UNOP: {
            char* operand = gen_expr(node->unop.operand);
            char* temp = gen_temp();
            emit(TAC_UNOP, temp, operand, NULL);
            free(operand);
            return temp;
        }
            
        case NODE_FUNC_CALL: {
            // Push parameters
            for (int i = 0; i < node->func_call.arg_count; i++) {
                char* arg = gen_expr(node->func_call.args[i]);
                emit(TAC_PARAM, NULL, arg, NULL);
                free(arg);
            }
            
            // Call function
            char* temp = gen_temp();
            char count_str[16];
            snprintf(count_str, sizeof(count_str), "%d", node->func_call.arg_count);
            emit(TAC_CALL, temp, node->func_call.name, count_str);
            return temp;
        }
            
        default:
            return NULL;
    }
}

// Generate TAC for statement
static void gen_stmt(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_STMT_LIST:
            for (int i = 0; i < node->list.count; i++) {
                gen_stmt(node->list.stmts[i]);
            }
            break;
            
        case NODE_ASSIGN: {
            char* val = gen_expr(node->assign.value);
            emit(TAC_COPY, node->assign.name, val, NULL);
            free(val);
            break;
        }
            
        case NODE_WRITE: {
            if (node->write_node.is_concat) {
                // String concatenation
                char str[1024];
                snprintf(str, sizeof(str), "\"%s\" + %s", 
                         node->write_node.prefix_str, 
                         node->write_node.suffix_id);
                emit(TAC_WRITE, NULL, str, NULL);
            } else {
                char* expr = gen_expr(node->write_node.expr);
                emit(TAC_WRITE, NULL, expr, NULL);
                free(expr);
            }
            break;
        }
            
        case NODE_READ: {
            char prompt[1024];
            snprintf(prompt, sizeof(prompt), "\"%s\"", node->read_node.prompt);
            emit(TAC_READ, node->read_node.var_name, prompt, NULL);
            break;
        }
            
        case NODE_IF: {
            char* cond = gen_expr(node->if_node.cond);
            char* Lelse = gen_label();
            char* Lend = gen_label();
            
            emit(TAC_IF_FALSE, NULL, cond, Lelse);
            free(cond);
            
            // Then body
            gen_stmt(node->if_node.then_body);
            emit(TAC_GOTO, NULL, Lend, NULL);
            
            // Elif chain
            char* current_else_label = Lelse;
            for (int i = 0; i < node->if_node.elif_count; i++) {
                emit_label(current_else_label);
                
                char* elif_cond = gen_expr(node->if_node.elif_conds[i]);
                char* next_label = (i < node->if_node.elif_count - 1 || node->if_node.else_body) 
                                   ? gen_label() : Lend;
                
                emit(TAC_IF_FALSE, NULL, elif_cond, next_label);
                free(elif_cond);
                
                gen_stmt(node->if_node.elif_bodies[i]);
                emit(TAC_GOTO, NULL, Lend, NULL);
                
                current_else_label = next_label;
            }
            
            // Else body
            if (node->if_node.else_body) {
                if (node->if_node.elif_count == 0) {
                    emit_label(Lelse);
                } else {
                    emit_label(current_else_label);
                }
                gen_stmt(node->if_node.else_body);
            } else if (node->if_node.elif_count == 0) {
                emit_label(Lelse);
            }
            
            emit_label(Lend);
            
            free(Lelse);
            free(Lend);
            break;
        }
            
        case NODE_LOOP_COUNTED: {
            // i = start
            char* start = gen_expr(node->loop_counted.start);
            emit(TAC_COPY, node->loop_counted.var, start, NULL);
            free(start);
            
            char* Lstart = gen_label();
            char* Lend = gen_label();
            
            emit_label(Lstart);
            
            // Check: i < end (or i > end for negative step)
            char* i_temp = gen_temp();
            emit(TAC_ASSIGN, i_temp, node->loop_counted.var, NULL);
            
            char* end = gen_expr(node->loop_counted.end);
            char* cond_temp = gen_temp();
            
            char op[8];
            if (node->loop_counted.step > 0) {
                snprintf(op, sizeof(op), "<");
            } else {
                snprintf(op, sizeof(op), ">");
            }
            
            emit(TAC_BINOP, cond_temp, i_temp, end);
            emit(TAC_IF_FALSE, NULL, cond_temp, Lend);
            
            free(i_temp);
            free(end);
            free(cond_temp);
            
            // Body
            gen_stmt(node->loop_counted.body);
            
            // i = i + step
            char* i_temp2 = gen_temp();
            emit(TAC_ASSIGN, i_temp2, node->loop_counted.var, NULL);
            
            char step_str[16];
            snprintf(step_str, sizeof(step_str), "%d", node->loop_counted.step);
            
            char* new_i = gen_temp();
            emit(TAC_BINOP, new_i, i_temp2, step_str);
            emit(TAC_COPY, node->loop_counted.var, new_i, NULL);
            
            free(i_temp2);
            free(new_i);
            
            emit(TAC_GOTO, NULL, Lstart, NULL);
            emit_label(Lend);
            
            free(Lstart);
            free(Lend);
            break;
        }
            
        case NODE_LOOP_INFINITE: {
            // Simplified: generate as infinite loop with check
            char* Lstart = gen_label();
            char* Lend = gen_label();
            
            emit_label(Lstart);
            
            // Body
            gen_stmt(node->loop_infinite.body);
            
            // Check condition
            char* check_start = gen_expr(node->loop_infinite.check_start);
            char* check_end = gen_expr(node->loop_infinite.check_end);
            
            char* cond = gen_temp();
            emit(TAC_BINOP, cond, check_start, check_end);
            emit(TAC_IF_FALSE, NULL, cond, Lend);
            
            free(check_start);
            free(check_end);
            free(cond);
            
            emit(TAC_GOTO, NULL, Lstart, NULL);
            emit_label(Lend);
            
            free(Lstart);
            free(Lend);
            break;
        }
            
        case NODE_FUNC_DEF: {
            // Function label
            char func_label[128];
            snprintf(func_label, sizeof(func_label), "FUNC_%s", node->func_def.name);
            emit_label(func_label);
            
            // Body
            gen_stmt(node->func_def.body);
            break;
        }
            
        case NODE_FUNC_CALL: {
            // Generate as statement (discard return value)
            char* result = gen_expr(node);
            free(result);
            break;
        }
            
        case NODE_RETURN: {
            if (node->ret_node.value) {
                char* val = gen_expr(node->ret_node.value);
                emit(TAC_RETURN, NULL, val, NULL);
                free(val);
            } else {
                emit(TAC_RETURN, NULL, NULL, NULL);
            }
            break;
        }
            
        default:
            break;
    }
}

// Initialize code generator
void codegen_init() {
    tac_head = tac_tail = NULL;
    temp_count = 0;
    label_count = 0;
}

// Generate TAC for entire program
void codegen_program(ASTNode* root) {
    if (!root) return;
    gen_stmt(root);
}

// Print TAC
void codegen_print() {
    printf("\n=== Three-Address Code (TAC) ===\n\n");
    
    TACInstr* instr = tac_head;
    while (instr) {
        switch (instr->op) {
            case TAC_ASSIGN:
                printf("  %s = %s\n", instr->result, instr->arg1);
                break;
            case TAC_BINOP:
                printf("  %s = %s op %s\n", instr->result, instr->arg1, instr->arg2);
                break;
            case TAC_UNOP:
                printf("  %s = -%s\n", instr->result, instr->arg1);
                break;
            case TAC_COPY:
                printf("  %s = %s\n", instr->result, instr->arg1);
                break;
            case TAC_LABEL:
                printf("%s:\n", instr->label);
                break;
            case TAC_GOTO:
                printf("  goto %s\n", instr->arg1);
                break;
            case TAC_IF_FALSE:
                printf("  if_false %s goto %s\n", instr->arg1, instr->arg2);
                break;
            case TAC_PARAM:
                printf("  param %s\n", instr->arg1);
                break;
            case TAC_CALL:
                printf("  %s = call %s %s\n", instr->result, instr->arg1, instr->arg2);
                break;
            case TAC_RETURN:
                if (instr->arg1) {
                    printf("  return %s\n", instr->arg1);
                } else {
                    printf("  return\n");
                }
                break;
            case TAC_WRITE:
                printf("  write %s\n", instr->arg1);
                break;
            case TAC_READ:
                printf("  read %s -> %s\n", instr->arg1, instr->result);
                break;
        }
        instr = instr->next;
    }
    
    printf("\n=== End of TAC ===\n\n");
}

// Free TAC
void codegen_free() {
    TACInstr* instr = tac_head;
    while (instr) {
        TACInstr* next = instr->next;
        free(instr->result);
        free(instr->arg1);
        free(instr->arg2);
        free(instr->label);
        free(instr);
        instr = next;
    }
    tac_head = tac_tail = NULL;
}
