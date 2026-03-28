%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>

int line_number = 1;  /* Track current line */
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
"if"        { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, line_number); }
"else"      { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, line_number); }
"while"     { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, line_number); }
"int"       { fprintf(output_file, "KEYWORD: %s (Line %d)\n", yytext, line_number); }

    /* OPERATORS - Multi-character operators MUST come before single-character */
">="        { fprintf(output_file, "OPERATOR: GREATER_THAN_OR_EQUAL (Line %d)\n", line_number); }
"<="        { fprintf(output_file, "OPERATOR: LESS_THAN_OR_EQUAL (Line %d)\n", line_number); }
"=="        { fprintf(output_file, "OPERATOR: EQUAL_TO (Line %d)\n", line_number); }
"!="        { fprintf(output_file, "OPERATOR: NOT_EQUAL (Line %d)\n", line_number); }
"+"         { fprintf(output_file, "OPERATOR: PLUS (Line %d)\n", line_number); }
"-"         { fprintf(output_file, "OPERATOR: MINUS (Line %d)\n", line_number); }
"*"         { fprintf(output_file, "OPERATOR: MULTIPLY (Line %d)\n", line_number); }
"="         { fprintf(output_file, "OPERATOR: ASSIGN (Line %d)\n", line_number); }
">"         { fprintf(output_file, "OPERATOR: GREATER_THAN (Line %d)\n", line_number); }
"<"         { fprintf(output_file, "OPERATOR: LESS_THAN (Line %d)\n", line_number); }

    /* DELIMITERS */
";"         { fprintf(output_file, "DELIMITER: SEMICOLON (Line %d)\n", line_number); }
","         { fprintf(output_file, "DELIMITER: COMMA (Line %d)\n", line_number); }
":"         { fprintf(output_file, "DELIMITER: COLON (Line %d)\n", line_number); }
"("         { fprintf(output_file, "DELIMITER: LPAREN (Line %d)\n", line_number); }
")"         { fprintf(output_file, "DELIMITER: RPAREN (Line %d)\n", line_number); }
"{"         { fprintf(output_file, "DELIMITER: LBRACE (Line %d)\n", line_number); }
"}"         { fprintf(output_file, "DELIMITER: RBRACE (Line %d)\n", line_number); }
"["         { fprintf(output_file, "DELIMITER: LBRACKET (Line %d)\n", line_number); }
"]"         { fprintf(output_file, "DELIMITER: RBRACKET (Line %d)\n", line_number); }

    /* STRING LITERALS */
\"[^\"]*\"  { fprintf(output_file, "STRING: %s (Line %d)\n", yytext, line_number); }

    /* SINGLE LINE COMMENT - Ignore everything after // until newline */
"//".*      { /* Ignore comment content */ }

    /* MULTI-LINE COMMENT - Enter comment state */
"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>\n             { line_number++; }
<COMMENT>.              { /* Ignore comment content */ }

    /* NUMBERS - Using our definition */
{NUMBER}    { fprintf(output_file, "NUMBER: %s (Line %d)\n", yytext, line_number); }

    /* IDENTIFIERS - Using our definition */
{IDENTIFIER} { fprintf(output_file, "IDENTIFIER: %s (Line %d)\n", yytext, line_number); }

    /* WHITESPACE - Just ignore spaces and tabs */
[ \t]+      { /* Do nothing - skip whitespace */ }

    /* NEWLINE - Count lines */
\n          { line_number++; }

    /* ANYTHING ELSE - Report as error */
.           { fprintf(output_file, "\tERROR: Unknown character '%s' (Line %d)\n", yytext, line_number); }

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
    printf("Processed %d lines from input.txt\n", line_number);
    
    return 0;
}
