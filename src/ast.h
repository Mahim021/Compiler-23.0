#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAM,
    NODE_STMT_LIST,
    NODE_ASSIGN,           // name = expr
    NODE_WRITE,            // write(expr) or write(str + var)
    NODE_READ,             // read("prompt":var)
    NODE_IF,               // if/elif/else chain
    NODE_ELIF,             // individual elif node
    NODE_LOOP_COUNTED,     // loop(i: step: start to end){ }
    NODE_LOOP_INFINITE,    // loop(i: 1){ }check(start to end)
    NODE_FUNC_DEF,         // name(params)[body]
    NODE_FUNC_CALL,        // name(args)
    NODE_RETURN,           // return expr
    NODE_BINOP,            // expr op expr
    NODE_UNOP,             // -expr
    NODE_NUMBER,           // numeric literal
    NODE_STRING,           // string literal
    NODE_IDENTIFIER,       // variable name
    NODE_PARAM_LIST,
    NODE_ARG_LIST,
    NODE_CONCAT,           // "str" + var (string concat)
} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    int line;
    
    union {
        // For literals
        double num_val;
        char*  str_val;
        char*  id_name;
        
        // For binop
        struct {
            char* op;
            ASTNode* left;
            ASTNode* right;
        } binop;
        
        // For unop
        struct {
            char* op;
            ASTNode* operand;
        } unop;
        
        // For assign
        struct {
            char* name;
            ASTNode* value;
        } assign;
        
        // For if statement
        struct {
            ASTNode* cond;
            ASTNode* then_body;
            ASTNode** elif_conds;
            ASTNode** elif_bodies;
            int elif_count;
            ASTNode* else_body;
        } if_node;
        
        // For loop_counted
        struct {
            char* var;
            int step;
            ASTNode* start;
            ASTNode* end;
            ASTNode* body;
        } loop_counted;
        
        // For loop_infinite
        struct {
            char* var;
            ASTNode* body;
            ASTNode* check_start;
            ASTNode* check_end;
        } loop_infinite;
        
        // For func_def
        struct {
            char* name;
            char** params;
            int param_count;
            ASTNode* body;
        } func_def;
        
        // For func_call
        struct {
            char* name;
            ASTNode** args;
            int arg_count;
        } func_call;
        
        // For write
        struct {
            ASTNode* expr;
            int is_concat;
            char* prefix_str;
            char* suffix_id;
        } write_node;
        
        // For read
        struct {
            char* prompt;
            char* var_name;
        } read_node;
        
        // For return
        struct {
            ASTNode* value;
        } ret_node;
        
        // For stmt_list / program
        struct {
            ASTNode** stmts;
            int count;
        } list;
    };
};

// Constructor functions
ASTNode* make_number(double val, int line);
ASTNode* make_string(char* val, int line);
ASTNode* make_identifier(char* name, int line);
ASTNode* make_assign(char* name, ASTNode* val, int line);
ASTNode* make_binop(char* op, ASTNode* left, ASTNode* right, int line);
ASTNode* make_unop(char* op, ASTNode* operand, int line);
ASTNode* make_write(ASTNode* expr, int line);
ASTNode* make_write_concat(char* prefix_str, char* suffix_id, int line);
ASTNode* make_read(char* prompt, char* var, int line);
ASTNode* make_if(ASTNode* cond, ASTNode* then_body,
                 ASTNode** elif_conds, ASTNode** elif_bodies,
                 int elif_count, ASTNode* else_body, int line);
ASTNode* make_loop_counted(char* var, int step,
                            ASTNode* start, ASTNode* end,
                            ASTNode* body, int line);
ASTNode* make_loop_infinite(char* var, ASTNode* body,
                             ASTNode* check_start, ASTNode* check_end,
                             int line);
ASTNode* make_func_def(char* name, char** params, int param_count,
                        ASTNode* body, int line);
ASTNode* make_func_call(char* name, ASTNode** args, int arg_count, int line);
ASTNode* make_return(ASTNode* val, int line);
ASTNode* make_stmt_list(ASTNode** stmts, int count, int line);
ASTNode* make_program(ASTNode* stmt_list, int line);

// Utility functions
void print_ast(ASTNode* node, int indent);
void free_ast(ASTNode* node);

#endif /* AST_H */
