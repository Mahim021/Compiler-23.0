%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/ast.h"

extern int yylineno;
extern FILE* yyin;
extern int yylex();

void yyerror(const char* s);

ASTNode* ast_root = NULL;
int parse_error_count = 0;

// Dynamic array helpers for building lists
typedef struct {
    ASTNode** items;
    int count;
    int capacity;
} NodeList;

static NodeList* nodelist_new() {
    NodeList* list = (NodeList*)malloc(sizeof(NodeList));
    list->count = 0;
    list->capacity = 8;
    list->items = (ASTNode**)malloc(sizeof(ASTNode*) * list->capacity);
    return list;
}

static void nodelist_add(NodeList* list, ASTNode* node) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (ASTNode**)realloc(list->items, sizeof(ASTNode*) * list->capacity);
    }
    list->items[list->count++] = node;
}

typedef struct {
    char** items;
    int count;
    int capacity;
} StrList;

static StrList* strlist_new() {
    StrList* list = (StrList*)malloc(sizeof(StrList));
    list->count = 0;
    list->capacity = 8;
    list->items = (char**)malloc(sizeof(char*) * list->capacity);
    return list;
}

static void strlist_add(StrList* list, char* str) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (char**)realloc(list->items, sizeof(char*) * list->capacity);
    }
    list->items[list->count++] = str;
}

%}

%union {
    double num_val;
    char* str_val;
    ASTNode* node;
    NodeList* node_list;
    StrList* str_list;
    int int_val;
}

%token <str_val> IF ELSE ELIF LOOP CHECK TO RETURN
%token <str_val> IDENTIFIER STRING
%token <num_val> NUMBER
%token PLUS MINUS STAR
%token EQ NEQ GTE LTE GT LT ASSIGN
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token SEMICOLON COMMA COLON

%type <node> program stmt_list stmt assignment_stmt write_stmt read_stmt
%type <node> if_stmt loop_stmt func_def_stmt return_stmt func_call_stmt
%type <node> expr func_call opt_else_clause
%type <node_list> stmt_list_builder elif_chain arg_list_builder
%type <str_list> param_list_builder
%type <int_val> step_expr

%left EQ NEQ
%left GT LT GTE LTE
%left PLUS MINUS
%left STAR
%right UMINUS
%left LPAREN

%%

program:
    IDENTIFIER LPAREN RPAREN LBRACKET stmt_list RBRACKET {
        if (strcmp($1, "start") != 0) {
            yyerror("Program must begin with start()[]");
            YYERROR;
        }
        ast_root = make_program($5, yylineno);
        free($1);
    }
    ;

stmt_list:
    /* empty */ {
        $$ = make_stmt_list(NULL, 0, yylineno);
    }
    | stmt_list_builder {
        $$ = make_stmt_list($1->items, $1->count, yylineno);
        free($1);
    }
    ;

stmt_list_builder:
    stmt {
        NodeList* list = nodelist_new();
        nodelist_add(list, $1);
        $$ = list;
    }
    | stmt_list_builder stmt {
        nodelist_add($1, $2);
        $$ = $1;
    }
    ;

stmt:
    assignment_stmt
    | write_stmt
    | read_stmt
    | if_stmt
    | loop_stmt
    | func_def_stmt
    | return_stmt
    | func_call_stmt
    ;

assignment_stmt:
    IDENTIFIER ASSIGN expr SEMICOLON {
        $$ = make_assign($1, $3, yylineno);
    }
    | IDENTIFIER ASSIGN expr {
        $$ = make_assign($1, $3, yylineno);
    }
    ;

write_stmt:
    IDENTIFIER LPAREN expr RPAREN SEMICOLON {
        if (strcmp($1, "write") != 0) {
            yyerror("Expected 'write' function");
            YYERROR;
        }
        $$ = make_write($3, yylineno);
        free($1);
    }
    | IDENTIFIER LPAREN expr RPAREN {
        if (strcmp($1, "write") != 0) {
            yyerror("Expected 'write' function");
            YYERROR;
        }
        $$ = make_write($3, yylineno);
        free($1);
    }
    | IDENTIFIER LPAREN STRING PLUS IDENTIFIER RPAREN SEMICOLON {
        if (strcmp($1, "write") != 0) {
            yyerror("Expected 'write' function");
            YYERROR;
        }
        $$ = make_write_concat($3, $5, yylineno);
        free($1);
    }
    | IDENTIFIER LPAREN STRING PLUS IDENTIFIER RPAREN {
        if (strcmp($1, "write") != 0) {
            yyerror("Expected 'write' function");
            YYERROR;
        }
        $$ = make_write_concat($3, $5, yylineno);
        free($1);
    }
    ;

read_stmt:
    IDENTIFIER LPAREN STRING COLON IDENTIFIER RPAREN SEMICOLON {
        if (strcmp($1, "read") != 0) {
            yyerror("Expected 'read' function");
            YYERROR;
        }
        $$ = make_read($3, $5, yylineno);
        free($1);
    }
    | IDENTIFIER LPAREN STRING COLON IDENTIFIER RPAREN {
        if (strcmp($1, "read") != 0) {
            yyerror("Expected 'read' function");
            YYERROR;
        }
        $$ = make_read($3, $5, yylineno);
        free($1);
    }
    ;

if_stmt:
    IF LPAREN expr RPAREN LBRACE stmt_list RBRACE elif_chain opt_else_clause {
        ASTNode** elif_conds = NULL;
        ASTNode** elif_bodies = NULL;
        int elif_count = 0;
        
        if ($8) {
            elif_count = $8->count / 2;
            elif_conds = (ASTNode**)malloc(sizeof(ASTNode*) * elif_count);
            elif_bodies = (ASTNode**)malloc(sizeof(ASTNode*) * elif_count);
            for (int i = 0; i < elif_count; i++) {
                elif_conds[i] = $8->items[i * 2];
                elif_bodies[i] = $8->items[i * 2 + 1];
            }
            free($8->items);
            free($8);
        }
        
        $$ = make_if($3, $6, elif_conds, elif_bodies, elif_count, $9, yylineno);
    }
    ;

elif_chain:
    /* empty */ {
        $$ = NULL;
    }
    | elif_chain ELIF LPAREN expr RPAREN LBRACE stmt_list RBRACE {
        if (!$1) {
            $1 = nodelist_new();
        }
        nodelist_add($1, $4);
        nodelist_add($1, $7);
        $$ = $1;
    }
    ;

opt_else_clause:
    /* empty */ {
        $$ = NULL;
    }
    | ELSE LBRACE stmt_list RBRACE {
        $$ = $3;
    }
    ;

loop_stmt:
    LOOP LPAREN IDENTIFIER COLON step_expr COLON expr TO expr RPAREN LBRACE stmt_list RBRACE {
        $$ = make_loop_counted($3, $5, $7, $9, $12, yylineno);
    }
    | LOOP LPAREN IDENTIFIER COLON NUMBER RPAREN LBRACE stmt_list RBRACE CHECK LPAREN expr TO expr RPAREN {
        $$ = make_loop_infinite($3, $8, $12, $14, yylineno);
    }
    ;

step_expr:
    PLUS NUMBER {
        $$ = (int)$2;
    }
    | MINUS NUMBER {
        $$ = -(int)$2;
    }
    | NUMBER {
        $$ = (int)$1;
    }
    ;

func_def_stmt:
    IDENTIFIER LPAREN param_list_builder RPAREN LBRACKET stmt_list RBRACKET {
        char** params = NULL;
        int param_count = 0;
        if ($3) {
            params = $3->items;
            param_count = $3->count;
            free($3);
        }
        $$ = make_func_def($1, params, param_count, $6, yylineno);
    }
    ;

param_list_builder:
    /* empty */ {
        $$ = NULL;
    }
    | IDENTIFIER {
        StrList* list = strlist_new();
        strlist_add(list, $1);
        $$ = list;
    }
    | param_list_builder COMMA IDENTIFIER {
        strlist_add($1, $3);
        $$ = $1;
    }
    ;

return_stmt:
    RETURN expr SEMICOLON {
        $$ = make_return($2, yylineno);
    }
    | RETURN expr {
        $$ = make_return($2, yylineno);
    }
    | RETURN SEMICOLON {
        $$ = make_return(NULL, yylineno);
    }
    | RETURN {
        $$ = make_return(NULL, yylineno);
    }
    ;

func_call_stmt:
    func_call SEMICOLON {
        $$ = $1;
    }
    | func_call {
        $$ = $1;
    }
    ;

func_call:
    IDENTIFIER LPAREN arg_list_builder RPAREN {
        ASTNode** args = NULL;
        int arg_count = 0;
        if ($3) {
            args = $3->items;
            arg_count = $3->count;
            free($3);
        }
        $$ = make_func_call($1, args, arg_count, yylineno);
    }
    ;

arg_list_builder:
    /* empty */ {
        $$ = NULL;
    }
    | expr {
        NodeList* list = nodelist_new();
        nodelist_add(list, $1);
        $$ = list;
    }
    | arg_list_builder COMMA expr {
        nodelist_add($1, $3);
        $$ = $1;
    }
    ;

expr:
    NUMBER {
        $$ = make_number($1, yylineno);
    }
    | STRING {
        $$ = make_string($1, yylineno);
        free($1);
    }
    | IDENTIFIER {
        $$ = make_identifier($1, yylineno);
        free($1);
    }
    | expr PLUS expr {
        $$ = make_binop("+", $1, $3, yylineno);
    }
    | expr MINUS expr {
        $$ = make_binop("-", $1, $3, yylineno);
    }
    | expr STAR expr {
        $$ = make_binop("*", $1, $3, yylineno);
    }
    | expr EQ expr {
        $$ = make_binop("==", $1, $3, yylineno);
    }
    | expr NEQ expr {
        $$ = make_binop("!=", $1, $3, yylineno);
    }
    | expr GTE expr {
        $$ = make_binop(">=", $1, $3, yylineno);
    }
    | expr LTE expr {
        $$ = make_binop("<=", $1, $3, yylineno);
    }
    | expr GT expr {
        $$ = make_binop(">", $1, $3, yylineno);
    }
    | expr LT expr {
        $$ = make_binop("<", $1, $3, yylineno);
    }
    | func_call {
        $$ = $1;
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    | MINUS expr %prec UMINUS {
        $$ = make_unop("-", $2, yylineno);
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Syntax Error at line %d: %s\n", yylineno, s);
    parse_error_count++;
}
