%option noyywrap
%option yylineno

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

/* For backward compatibility with original lexer */
FILE *output_file = NULL;
%}

DIGIT       [0-9]
LETTER      [a-zA-Z]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
NUMBER      {DIGIT}+

/* Start condition for multi-line comments */
%x COMMENT

%%

    /* KEYWORDS - Must come before IDENTIFIER rule */
"if"        { return IF; }
"else"      { return ELSE; }
"elif"      { return ELIF; }
"loop"      { return LOOP; }
"check"     { return CHECK; }
"to"        { return TO; }
"return"    { return RETURN; }

    /* OPERATORS - Multi-character operators MUST come before single-character */
">="        { return GTE; }
"<="        { return LTE; }
"=="        { return EQ; }
"!="        { return NEQ; }
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return STAR; }
"="         { return ASSIGN; }
">"         { return GT; }
"<"         { return LT; }

    /* DELIMITERS */
";"         { return SEMICOLON; }
","         { return COMMA; }
":"         { return COLON; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
"["         { return LBRACKET; }
"]"         { return RBRACKET; }

    /* STRING LITERALS */
\"[^\"]*\"  { 
    yylval.str_val = strndup(yytext + 1, strlen(yytext) - 2);  /* Remove quotes */
    return STRING; 
}

    /* SINGLE LINE COMMENT */
"//".*      { /* Ignore comment content */ }

    /* MULTI-LINE COMMENT */
"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>\n             { /* yylineno auto-increments */ }
<COMMENT>.              { /* Ignore comment content */ }

    /* NUMBERS */
{NUMBER}    { 
    yylval.num_val = atof(yytext); 
    return NUMBER; 
}

    /* IDENTIFIERS */
{IDENTIFIER} { 
    yylval.str_val = strdup(yytext); 
    return IDENTIFIER; 
}

    /* WHITESPACE */
[ \t]+      { /* Do nothing - skip whitespace */ }

    /* NEWLINE */
\n          { /* yylineno handles this automatically */ }

    /* ANYTHING ELSE */
.           { 
    fprintf(stderr, "Lexical Error at line %d: Unknown character '%s'\n", 
            yylineno, yytext); 
}

%%
