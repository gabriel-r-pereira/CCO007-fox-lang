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
%token <d> NUMBER
%token <s> ID
%token <fn> FUNC
%token IF THEN ELSE WHILE DO DEF
%token EOL

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%nonassoc NO_ELSE
%nonassoc ELSE

%type <a> exp stmt if_stmt stmt_list explist assigment u_function b_function
%type <sl> symlist

%start lines

%%

stmt: if_stmt
    | assigment
    | u_function
    | b_function
    ;


if_stmt: IF exp stmt %prec NO_ELSE                     { $$ = newflow('I', $2, $3, NULL); }
    | IF exp '{' stmt_list '}' %prec NO_ELSE           { $$ = newflow('I', $2, $4, NULL); }
    | IF exp stmt ELSE stmt                            { $$ = newflow('I', $2, $3, $5); }
    | IF exp '{' stmt_list '}' ELSE '{' stmt_list '}'  { $$ = newflow('I', $2, $4, $8); }
    ;

stmt_list: /* vazio */  { $$ = NULL; }
    | stmt stmt_list {
        if ($2 == NULL)
            $$ = $1;
        else
            $$ = newast('L', $1, $2);
    }
    ;

assigment: ID '=' exp ';'               { $$ = newasgn($1, $3); }
    ;
u_function: FUNC '(' explist ')' ';'    { $$ = newfunc($1, $3); }
    ;
b_function: ID '(' explist ')' ';'     { $$ = newcall($1, $3); }
    ;

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

explist: exp
    | exp ',' explist       { $$ = newast('L', $1, $3); }
    ;

symlist: ID                 { $$ = newsymlist($1, NULL); }
    | ID ',' symlist        { $$ = newsymlist($1, $3); }
    ;

lines: /* comentário ou linha vazia */
    | lines EOL
    | lines stmt {
        eval($2);
        treefree($2);
    }
    | lines DEF ID '(' symlist ')' '{' stmt_list '}' EOL {
        dodef($3, $5, $8);
        fprintf(yyout, "Defined %s\n", $3->name);
    }
    | lines error EOL {
        yyerrok;
    }
    ;
%%
