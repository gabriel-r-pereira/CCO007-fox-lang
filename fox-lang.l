%option noyywrap nodefault yylineno
%{
#include "fox-lang.h"
#include "fox-lang.tab.h"
%}

/* Expoente de um número fracionário */
EXP ([Ee][-+]?[0-9]+)

%%

 /* Operações de somente um caractere */
"+" |
"-" |
"*" |
"/" |
"|" |
"=" |
"," |
";" |
"(" |
")"     { return yytext[0]; } /* retorna o char */

 /* Operações de comparação, todos usam o token CMP */
">"     { yylval.fn = 1; return CMP; }
"<"     { yylval.fn = 2; return CMP; }
"!="    { yylval.fn = 3; return CMP; }
"=="    { yylval.fn = 4; return CMP; }
">="    { yylval.fn = 5; return CMP; }
"<="    { yylval.fn = 6; return CMP; }

 /* Palavras reservadas */
"if"    { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"while" { return WHILE; }
"do"    { return DO; }
"let"   { return LET; }

 /* Funções da linguagem */
 "sqrt"     { yylval.fn = B_sqrt; return FUNC; }
 "exp"      { yylval.fn = B_exp; return FUNC; }
 "log"      { yylval.fn = B_log; return FUNC; }
 "print"    { yylval.fn = B_print; return FUNC; }

 /* Identificadores */
[a-zA-Z][a-zA-Z0-9_]* {
    yylval.s = lookup(yytext);
    return ID;
}

 /* Números */
[0-9]+"."[0-9]*{EXP}?|"."?[0-9]+{EXP}? {
    yylval.d = atof(yytext);
    return NUMBER;
}

"//".*  // ignora comentários da forma "//"
[ \t]   /* ignora espaços em branco */

\\\n    { printf ("c> "); } /* permite a continuação da declaração na linha seguinte */
\n      { return EOL; }
.       { yyerror("Mystery character %c\n", *yytext); }

%%
