#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

#define HASH_SIZE 128

// Hash table node
typedef struct HashNode {
    Symbol symbol;
    struct HashNode* next;
} HashNode;

// Scope (one hash table)
typedef struct Scope {
    HashNode* table[HASH_SIZE];
    struct Scope* parent;
} Scope;

// Global scope stack
static Scope* current_scope = NULL;
static Scope* global_scope = NULL;

// Hash function
static unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

// Initialize symbol table
void symtable_init() {
    global_scope = (Scope*)malloc(sizeof(Scope));
    memset(global_scope, 0, sizeof(Scope));
    global_scope->parent = NULL;
    current_scope = global_scope;
}

// Destroy symbol table
void symtable_destroy() {
    while (current_scope) {
        scope_pop();
    }
}

// Push a new scope
void scope_push() {
    Scope* new_scope = (Scope*)malloc(sizeof(Scope));
    memset(new_scope, 0, sizeof(Scope));
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

// Pop current scope
void scope_pop() {
    if (!current_scope) return;
    
    Scope* old_scope = current_scope;
    current_scope = current_scope->parent;
    
    // Free all symbols in the scope
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* node = old_scope->table[i];
        while (node) {
            HashNode* next = node->next;
            free(node->symbol.name);
            free_value(&node->symbol.value);
            free(node);
            node = next;
        }
    }
    
    free(old_scope);
}

// Set symbol in current scope
void symbol_set(char* name, Value val, int line) {
    if (!current_scope) {
        fprintf(stderr, "Fatal: No scope available\n");
        exit(1);
    }
    
    unsigned int idx = hash(name);
    
    // Check if symbol already exists in current scope
    HashNode* node = current_scope->table[idx];
    while (node) {
        if (strcmp(node->symbol.name, name) == 0) {
            // Update existing symbol
            free_value(&node->symbol.value);
            node->symbol.value = copy_value(&val);
            return;
        }
        node = node->next;
    }
    
    // Create new symbol
    HashNode* new_node = (HashNode*)malloc(sizeof(HashNode));
    new_node->symbol.name = strdup(name);
    new_node->symbol.value = copy_value(&val);
    new_node->symbol.line_declared = line;
    new_node->next = current_scope->table[idx];
    current_scope->table[idx] = new_node;
}

// Get symbol (search up scope chain)
Value* symbol_get(char* name) {
    Scope* scope = current_scope;
    
    while (scope) {
        unsigned int idx = hash(name);
        HashNode* node = scope->table[idx];
        
        while (node) {
            if (strcmp(node->symbol.name, name) == 0) {
                return &node->symbol.value;
            }
            node = node->next;
        }
        
        scope = scope->parent;
    }
    
    return NULL;  // Not found
}

// Check if symbol exists in current scope only
int symbol_exists_local(char* name) {
    if (!current_scope) return 0;
    
    unsigned int idx = hash(name);
    HashNode* node = current_scope->table[idx];
    
    while (node) {
        if (strcmp(node->symbol.name, name) == 0) {
            return 1;
        }
        node = node->next;
    }
    
    return 0;
}

// Check if symbol exists in any scope
int symbol_exists(char* name) {
    return symbol_get(name) != NULL;
}

// Value constructors
Value make_value_number(double num) {
    Value val;
    val.type = TYPE_NUMBER;
    val.num = num;
    return val;
}

Value make_value_string(char* str) {
    Value val;
    val.type = TYPE_STRING;
    val.str = strdup(str);
    return val;
}

Value make_value_bool(int boolean) {
    Value val;
    val.type = TYPE_BOOL;
    val.boolean = boolean;
    return val;
}

Value make_value_func(char** params, int param_count, ASTNode* body) {
    Value val;
    val.type = TYPE_FUNC;
    val.func.param_count = param_count;
    
    if (param_count > 0) {
        val.func.params = (char**)malloc(sizeof(char*) * param_count);
        for (int i = 0; i < param_count; i++) {
            val.func.params[i] = strdup(params[i]);
        }
    } else {
        val.func.params = NULL;
    }
    
    val.func.body = body;
    return val;
}

Value make_value_null() {
    Value val;
    val.type = TYPE_NULL;
    return val;
}

// Free value contents
void free_value(Value* val) {
    if (!val) return;
    
    switch (val->type) {
        case TYPE_STRING:
            free(val->str);
            break;
        case TYPE_FUNC:
            for (int i = 0; i < val->func.param_count; i++) {
                free(val->func.params[i]);
            }
            if (val->func.param_count > 0) {
                free(val->func.params);
            }
            // Note: don't free body - it's owned by AST
            break;
        default:
            break;
    }
}

// Copy value
Value copy_value(Value* val) {
    if (!val) return make_value_null();
    
    Value copy;
    copy.type = val->type;
    
    switch (val->type) {
        case TYPE_NUMBER:
            copy.num = val->num;
            break;
        case TYPE_STRING:
            copy.str = strdup(val->str);
            break;
        case TYPE_BOOL:
            copy.boolean = val->boolean;
            break;
        case TYPE_FUNC:
            copy.func.param_count = val->func.param_count;
            if (val->func.param_count > 0) {
                copy.func.params = (char**)malloc(sizeof(char*) * val->func.param_count);
                for (int i = 0; i < val->func.param_count; i++) {
                    copy.func.params[i] = strdup(val->func.params[i]);
                }
            } else {
                copy.func.params = NULL;
            }
            copy.func.body = val->func.body;
            break;
        case TYPE_NULL:
            break;
    }
    
    return copy;
}

// Convert value to string representation
char* value_to_string(Value* val) {
    static char buffer[256];
    
    if (!val) {
        snprintf(buffer, sizeof(buffer), "null");
        return buffer;
    }
    
    switch (val->type) {
        case TYPE_NUMBER:
            // Print as integer if whole number
            if (val->num == (int)val->num) {
                snprintf(buffer, sizeof(buffer), "%d", (int)val->num);
            } else {
                snprintf(buffer, sizeof(buffer), "%.10g", val->num);
            }
            break;
        case TYPE_STRING:
            return val->str;
        case TYPE_BOOL:
            snprintf(buffer, sizeof(buffer), "%s", val->boolean ? "true" : "false");
            break;
        case TYPE_FUNC:
            snprintf(buffer, sizeof(buffer), "<function>");
            break;
        case TYPE_NULL:
            snprintf(buffer, sizeof(buffer), "null");
            break;
    }
    
    return buffer;
}

// Check if value is truthy
int value_is_truthy(Value* val) {
    if (!val) return 0;
    
    switch (val->type) {
        case TYPE_NUMBER:
            return val->num != 0.0;
        case TYPE_STRING:
            return val->str && strlen(val->str) > 0;
        case TYPE_BOOL:
            return val->boolean;
        case TYPE_FUNC:
            return 1;
        case TYPE_NULL:
            return 0;
    }
    
    return 0;
}
