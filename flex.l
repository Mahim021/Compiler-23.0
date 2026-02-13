%option noyywrap
%option yylineno

%{
#include <stdio.h>
#include <stdlib.h>

FILE *output_file;    /* Output file pointer */
%}

DIGIT       [0-9]
LETTER      [a-zA-Z]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
NUMBER      {DIGIT}+

/* Start condition for multi-line comments */
%x COMMENT

%%

    /* KEYWORDS - Must come before IDENTIFIER rule */
"if"        { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, yylineno); }
"else"      { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, yylineno); }
"elif"      { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, yylineno); }
"loop"      { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, yylineno); }
"check"     { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, yylineno); }
"to"        { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, yylineno); }
"return"    { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, yylineno); }

    /* OPERATORS - Multi-character operators MUST come before single-character */
">="        { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"<="        { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"=="        { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"!="        { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"+"         { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"-"         { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"*"         { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"="         { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
">"         { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }
"<"         { fprintf(output_file, "OPERATOR: %s (Line %d)\n", yytext, yylineno); }

    /* DELIMITERS */
";"         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
","         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
":"         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
"("         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
")"         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
"{"         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
"}"         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
"["         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }
"]"         { fprintf(output_file, "DELIMITER: %s (Line %d)\n", yytext, yylineno); }

\"[^\"]*\"  { fprintf(output_file, "STRING: %s (Line %d)\n", yytext, yylineno); }

    /* SINGLE LINE COMMENT */
"//".*      { /* Ignore comment content */ }

    /* MULTI-LINE COMMENT */
"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>\n             { /* yylineno auto-increments */ }
<COMMENT>.              { /* Ignore comment content */ }

    /* NUMBERS */
{NUMBER}    { fprintf(output_file, "NUMBER: %s (Line %d)\n", yytext, yylineno); }

    /* IDENTIFIERS */
{IDENTIFIER} { fprintf(output_file, "IDENTIFIER: %s (Line %d)\n", yytext, yylineno); }

    /* WHITESPACE */
[ \t]+      { /* Do nothing - skip whitespace */ }

    /* NEWLINE */
\n          { /* yylineno handles this automatically */ }

    /* ANYTHING ELSE */
.           { fprintf(output_file, "ERROR: Unknown character '%s' (Line %d)\n", yytext, yylineno); }

%%

int main() {
    FILE *input_file = fopen("input.txt", "r");
    if (!input_file) {
        printf("ERROR: Cannot open input.txt\n");
        printf("Make sure input.txt exists in the same folder.\n");
        return 1;
    }

    output_file = fopen("output.txt", "w");
    if (!output_file) {
        printf("ERROR: Cannot create output.txt\n");
        fclose(input_file);
        return 1;
    }

    yyin = input_file;

    yylex();

    fclose(input_file);
    fclose(output_file);
    
    printf("SUCCESS! Tokens have been written to output.txt\n");
    printf("Processed %d lines from input.txt\n", yylineno);
    
    return 0;
}
