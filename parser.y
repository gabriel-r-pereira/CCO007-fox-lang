%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
%}

%union {
    struct ast *a;
    double d;
    struct symbol *s;
    struct symlist *sl;
    int fn;             // função
}

/* Declaração dos tokens */
%token <d> NUMBER   // número
%token <s> ID       // identificador
%token <fn> FUNC    // função da linguagem
%token IF ELSE      // if/else
%token WHILE        // while
%token DEF          // palavra reservada para definição de funções
%token EOL          // End-Of-File

/* Precedência dos operadores */
%nonassoc <fn> CMP  // comparadores
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%nonassoc NO_ELSE
%nonassoc ELSE

/* Tipo de cada símbolo não-terminal */
%type <a> stmt stmt_list
%type <a> exp explist
%type <a> if_stmt while_stmt
%type <a> assigment u_function b_function
%type <sl> symlist

/* Símbolo inicial */
%start lines

%%

/* Declarações */
stmt: if_stmt
    | while_stmt
    | assigment
    | u_function
    | b_function
    ;

/* if/else */
if_stmt: IF exp stmt %prec NO_ELSE {    // if, única declaração
        $$ = newflow('I', $2, $3, NULL);
    }
    | IF exp stmt ELSE stmt {           // if/else, única declaração
        $$ = newflow('I', $2, $3, $5);
    }
    | IF exp '{' stmt_list '}' %prec NO_ELSE {          // if, múltiplas declarações
        $$ = newflow('I', $2, $4, NULL);
    }
    | IF exp '{' stmt_list '}' ELSE '{' stmt_list '}' { // if/else, múltiplas declarações
        $$ = newflow('I', $2, $4, $8);
    }
    ;

while_stmt: WHILE exp stmt {                // while, única declaração
        $$ = newflow('W', $2, $3, NULL);
    }
    | WHILE exp '{' stmt_list '}' {         // while, múltiplas declarações
        $$ = newflow('W', $2, $4, NULL);
    }
    ;

/* Lista de declarações */
stmt_list: /* vazio */  { $$ = NULL; }
    | stmt stmt_list {
        if ($2 == NULL)
            $$ = $1;
        else
            $$ = newast('L', $1, $2);
    }
    ;

/* Atribuição de variáveis */
assigment: ID '=' exp ';'               { $$ = newasgn($1, $3); }
    ;
/* Chamada de funções definidas pelo usuário */
u_function: FUNC '(' explist ')' ';'    { $$ = newfunc($1, $3); }
    ;
/* Chamada de funções da linguagem */
b_function: ID '(' explist ')' ';'      { $$ = newcall($1, $3); }
    ;

/* Expressão */
exp: exp CMP exp            { $$ = newcmp($2, $1, $3); }
    | exp '+' exp           { $$ = newast('+', $1, $3); }
    | exp '-' exp           { $$ = newast('-', $1, $3); }
    | exp '*' exp           { $$ = newast('*', $1, $3); }
    | exp '/' exp           { $$ = newast('/', $1, $3); }
    | '(' exp ')'           { $$ = $2; }
    | '-' exp %prec UMINUS  { $$ = newast('M', $2, NULL); }
    | NUMBER                { $$ = newnum($1); }
    | ID                    { $$ = newref($1); }
    | assigment
    | u_function
    | b_function
    ;

/* Lista de expressões */
explist: exp
    | exp ',' explist       { $$ = newast('L', $1, $3); }
    ;

/* Lista de símbolos */
symlist: ID                 { $$ = newsymlist($1, NULL); }
    | ID ',' symlist        { $$ = newsymlist($1, $3); }
    ;

/* Linhas da entrada */
lines: /* comentário ou espaço(s) em branco */
    | lines EOL     // linhas vazias
    | lines stmt {  // declaração
        eval($2);
        treefree($2);
    }
    | lines DEF ID '(' symlist ')' '{' stmt_list '}' EOL { // definição de função
        dodef($3, $5, $8);
        fprintf(yyout, "Defined %s\n", $3->name);
    }
    | lines error EOL { // erro no reconhecimento da linguagem
        yyerrok;
    }
    ;
%%
