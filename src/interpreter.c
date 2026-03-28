#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter.h"

int return_flag = 0;
Value return_value;

static void runtime_error(int line, const char* msg) {
    fprintf(stderr, "Runtime Error [line %d]: %s\n", line, msg);
    exit(1);
}

// Forward declarations
static void exec_stmt(ASTNode* node);
static Value eval_expr(ASTNode* node);

// Execute statement
static void exec_stmt(ASTNode* node) {
    if (!node) return;
    
    // Check for return flag
    if (return_flag) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_STMT_LIST:
            for (int i = 0; i < node->list.count; i++) {
                exec_stmt(node->list.stmts[i]);
                if (return_flag) return;
            }
            break;
            
        case NODE_ASSIGN: {
            Value val = eval_expr(node->assign.value);
            symbol_set(node->assign.name, val, node->line);
            free_value(&val);
            break;
        }
            
        case NODE_WRITE: {
            if (node->write_node.is_concat) {
                // String concatenation: "str" + var
                Value* var_val = symbol_get(node->write_node.suffix_id);
                if (!var_val) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Undefined variable '%s'", 
                             node->write_node.suffix_id);
                    runtime_error(node->line, msg);
                }
                printf("%s%s\n", node->write_node.prefix_str, value_to_string(var_val));
            } else {
                Value val = eval_expr(node->write_node.expr);
                printf("%s\n", value_to_string(&val));
                free_value(&val);
            }
            break;
        }
            
        case NODE_READ: {
            printf("%s", node->read_node.prompt);
            fflush(stdout);
            
            char buffer[1024];
            if (fgets(buffer, sizeof(buffer), stdin)) {
                // Remove newline
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n') {
                    buffer[len - 1] = '\0';
                }
                
                // Try to parse as number
                char* endptr;
                double num = strtod(buffer, &endptr);
                
                Value val;
                if (*endptr == '\0' && *buffer != '\0') {
                    // Successfully parsed as number
                    val = make_value_number(num);
                } else {
                    // Store as string
                    val = make_value_string(buffer);
                }
                
                symbol_set(node->read_node.var_name, val, node->line);
                free_value(&val);
            }
            break;
        }
            
        case NODE_IF: {
            Value cond = eval_expr(node->if_node.cond);
            int is_true = value_is_truthy(&cond);
            free_value(&cond);
            
            if (is_true) {
                exec_stmt(node->if_node.then_body);
            } else {
                // Check elif conditions
                int matched = 0;
                for (int i = 0; i < node->if_node.elif_count; i++) {
                    Value elif_cond = eval_expr(node->if_node.elif_conds[i]);
                    int elif_true = value_is_truthy(&elif_cond);
                    free_value(&elif_cond);
                    
                    if (elif_true) {
                        exec_stmt(node->if_node.elif_bodies[i]);
                        matched = 1;
                        break;
                    }
                }
                
                if (!matched && node->if_node.else_body) {
                    exec_stmt(node->if_node.else_body);
                }
            }
            break;
        }
            
        case NODE_LOOP_COUNTED: {
            Value start_val = eval_expr(node->loop_counted.start);
            Value end_val = eval_expr(node->loop_counted.end);
            
            if (start_val.type != TYPE_NUMBER || end_val.type != TYPE_NUMBER) {
                runtime_error(node->line, "Loop bounds must be numbers");
            }
            
            double i = start_val.num;
            double end = end_val.num;
            int step = node->loop_counted.step;
            
            free_value(&start_val);
            free_value(&end_val);
            
            // Create loop variable
            scope_push();
            
            while ((step > 0 && i < end) || (step < 0 && i > end)) {
                Value loop_var = make_value_number(i);
                symbol_set(node->loop_counted.var, loop_var, node->line);
                free_value(&loop_var);
                
                exec_stmt(node->loop_counted.body);
                
                if (return_flag) {
                    scope_pop();
                    return;
                }
                
                i += step;
            }
            
            scope_pop();
            break;
        }
            
        case NODE_LOOP_INFINITE: {
            Value start_val = eval_expr(node->loop_infinite.check_start);
            Value end_val = eval_expr(node->loop_infinite.check_end);
            
            if (start_val.type != TYPE_NUMBER || end_val.type != TYPE_NUMBER) {
                runtime_error(node->line, "Check bounds must be numbers");
            }
            
            double check_start = start_val.num;
            double check_end = end_val.num;
            
            free_value(&start_val);
            free_value(&end_val);
            
            scope_push();
            
            double i = 1;
            while (1) {
                Value loop_var = make_value_number(i);
                symbol_set(node->loop_infinite.var, loop_var, node->line);
                free_value(&loop_var);
                
                exec_stmt(node->loop_infinite.body);
                
                if (return_flag) {
                    scope_pop();
                    return;
                }
                
                i++;
                
                // Check termination: NOT (i >= check_start && i < check_end)
                if (!(i >= check_start && i < check_end)) {
                    break;
                }
            }
            
            scope_pop();
            break;
        }
            
        case NODE_FUNC_DEF: {
            // Define function in symbol table
            Value func_val = make_value_func(node->func_def.params, 
                                              node->func_def.param_count, 
                                              node->func_def.body);
            symbol_set(node->func_def.name, func_val, node->line);
            free_value(&func_val);
            break;
        }
            
        case NODE_FUNC_CALL: {
            // Execute function call as statement (discard return value)
            Value result = eval_expr(node);
            free_value(&result);
            break;
        }
            
        case NODE_RETURN: {
            if (node->ret_node.value) {
                return_value = eval_expr(node->ret_node.value);
            } else {
                return_value = make_value_null();
            }
            return_flag = 1;
            break;
        }
            
        default:
            break;
    }
}

// Evaluate expression
static Value eval_expr(ASTNode* node) {
    if (!node) return make_value_null();
    
    switch (node->type) {
        case NODE_NUMBER:
            return make_value_number(node->num_val);
            
        case NODE_STRING:
            return make_value_string(node->str_val);
            
        case NODE_IDENTIFIER: {
            Value* val = symbol_get(node->id_name);
            if (!val) {
                char msg[256];
                snprintf(msg, sizeof(msg), "Undefined variable '%s'", node->id_name);
                runtime_error(node->line, msg);
            }
            return copy_value(val);
        }
            
        case NODE_BINOP: {
            Value left = eval_expr(node->binop.left);
            Value right = eval_expr(node->binop.right);
            Value result;
            
            if (strcmp(node->binop.op, "+") == 0) {
                if (left.type == TYPE_NUMBER && right.type == TYPE_NUMBER) {
                    result = make_value_number(left.num + right.num);
                } else if (left.type == TYPE_STRING || right.type == TYPE_STRING) {
                    // String concatenation
                    char buffer[1024];
                    snprintf(buffer, sizeof(buffer), "%s%s", 
                             value_to_string(&left), value_to_string(&right));
                    result = make_value_string(buffer);
                } else {
                    runtime_error(node->line, "Cannot add these types");
                }
            } else if (strcmp(node->binop.op, "-") == 0) {
                if (left.type != TYPE_NUMBER || right.type != TYPE_NUMBER) {
                    runtime_error(node->line, "Cannot subtract non-numbers");
                }
                result = make_value_number(left.num - right.num);
            } else if (strcmp(node->binop.op, "*") == 0) {
                if (left.type != TYPE_NUMBER || right.type != TYPE_NUMBER) {
                    runtime_error(node->line, "Cannot multiply non-numbers");
                }
                result = make_value_number(left.num * right.num);
            } else if (strcmp(node->binop.op, "==") == 0) {
                int equal = 0;
                if (left.type == TYPE_NUMBER && right.type == TYPE_NUMBER) {
                    equal = (left.num == right.num);
                } else if (left.type == TYPE_STRING && right.type == TYPE_STRING) {
                    equal = (strcmp(left.str, right.str) == 0);
                }
                result = make_value_number(equal ? 1.0 : 0.0);
            } else if (strcmp(node->binop.op, "!=") == 0) {
                int not_equal = 0;
                if (left.type == TYPE_NUMBER && right.type == TYPE_NUMBER) {
                    not_equal = (left.num != right.num);
                } else if (left.type == TYPE_STRING && right.type == TYPE_STRING) {
                    not_equal = (strcmp(left.str, right.str) != 0);
                }
                result = make_value_number(not_equal ? 1.0 : 0.0);
            } else if (strcmp(node->binop.op, ">") == 0) {
                if (left.type != TYPE_NUMBER || right.type != TYPE_NUMBER) {
                    runtime_error(node->line, "Cannot compare non-numbers");
                }
                result = make_value_number((left.num > right.num) ? 1.0 : 0.0);
            } else if (strcmp(node->binop.op, "<") == 0) {
                if (left.type != TYPE_NUMBER || right.type != TYPE_NUMBER) {
                    runtime_error(node->line, "Cannot compare non-numbers");
                }
                result = make_value_number((left.num < right.num) ? 1.0 : 0.0);
            } else if (strcmp(node->binop.op, ">=") == 0) {
                if (left.type != TYPE_NUMBER || right.type != TYPE_NUMBER) {
                    runtime_error(node->line, "Cannot compare non-numbers");
                }
                result = make_value_number((left.num >= right.num) ? 1.0 : 0.0);
            } else if (strcmp(node->binop.op, "<=") == 0) {
                if (left.type != TYPE_NUMBER || right.type != TYPE_NUMBER) {
                    runtime_error(node->line, "Cannot compare non-numbers");
                }
                result = make_value_number((left.num <= right.num) ? 1.0 : 0.0);
            } else {
                runtime_error(node->line, "Unknown binary operator");
            }
            
            free_value(&left);
            free_value(&right);
            return result;
        }
            
        case NODE_UNOP: {
            Value operand = eval_expr(node->unop.operand);
            Value result;
            
            if (strcmp(node->unop.op, "-") == 0) {
                if (operand.type != TYPE_NUMBER) {
                    runtime_error(node->line, "Cannot negate non-number");
                }
                result = make_value_number(-operand.num);
            } else {
                runtime_error(node->line, "Unknown unary operator");
            }
            
            free_value(&operand);
            return result;
        }
            
        case NODE_FUNC_CALL: {
            Value* func_val = symbol_get(node->func_call.name);
            if (!func_val || func_val->type != TYPE_FUNC) {
                char msg[256];
                snprintf(msg, sizeof(msg), "Function '%s' is not defined", 
                         node->func_call.name);
                runtime_error(node->line, msg);
            }
            
            if (func_val->func.param_count != node->func_call.arg_count) {
                char msg[256];
                snprintf(msg, sizeof(msg), 
                         "Argument count mismatch calling '%s': expected %d got %d",
                         node->func_call.name, func_val->func.param_count, 
                         node->func_call.arg_count);
                runtime_error(node->line, msg);
            }
            
            // Evaluate arguments
            Value* arg_values = NULL;
            if (node->func_call.arg_count > 0) {
                arg_values = (Value*)malloc(sizeof(Value) * node->func_call.arg_count);
                for (int i = 0; i < node->func_call.arg_count; i++) {
                    arg_values[i] = eval_expr(node->func_call.args[i]);
                }
            }
            
            // Push new scope
            scope_push();
            
            // Bind parameters
            for (int i = 0; i < func_val->func.param_count; i++) {
                symbol_set(func_val->func.params[i], arg_values[i], node->line);
            }
            
            // Free argument values (symbol_set makes copies)
            for (int i = 0; i < node->func_call.arg_count; i++) {
                free_value(&arg_values[i]);
            }
            free(arg_values);
            
            // Execute function body
            return_flag = 0;
            exec_stmt(func_val->func.body);
            
            Value result;
            if (return_flag) {
                result = copy_value(&return_value);
                free_value(&return_value);
                return_flag = 0;
            } else {
                result = make_value_null();
            }
            
            // Pop scope
            scope_pop();
            
            return result;
        }
            
        default:
            return make_value_null();
    }
}

// Main interpreter entry point
void interpret(ASTNode* root) {
    if (!root) return;
    
    symtable_init();
    return_flag = 0;
    
    exec_stmt(root);
    
    symtable_destroy();
}

// Export for external use
Value eval_expr(ASTNode* node) {
    return eval_expr(node);
}

void exec_stmt(ASTNode* node) {
    exec_stmt(node);
}
