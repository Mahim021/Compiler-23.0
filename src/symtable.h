#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"

// Value types for dynamic typing
typedef enum {
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_FUNC,
    TYPE_NULL
} ValType;

// Runtime value
typedef struct Value {
    ValType type;
    union {
        double num;        // for TYPE_NUMBER
        char* str;         // for TYPE_STRING
        int boolean;       // for TYPE_BOOL
        struct {           // for TYPE_FUNC
            char** params;
            int param_count;
            ASTNode* body;
        } func;
    };
} Value;

// Symbol table entry
typedef struct Symbol {
    char* name;
    Value value;
    int line_declared;
} Symbol;

// Symbol table operations
void    symtable_init();
void    symtable_destroy();
void    scope_push();
void    scope_pop();
void    symbol_set(char* name, Value val, int line);
Value*  symbol_get(char* name);
int     symbol_exists_local(char* name);
int     symbol_exists(char* name);

// Value utility functions
Value   make_value_number(double num);
Value   make_value_string(char* str);
Value   make_value_bool(int boolean);
Value   make_value_func(char** params, int param_count, ASTNode* body);
Value   make_value_null();
void    free_value(Value* val);
Value   copy_value(Value* val);
char*   value_to_string(Value* val);
int     value_is_truthy(Value* val);

#endif /* SYMTABLE_H */
