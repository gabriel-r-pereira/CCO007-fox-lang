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
%token IF THEN ELSE WHILE DO LET
%token EOL

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS

%type <a> exp stmt list explist
%type <sl> symlist

%start calclist

%%

stmt: IF exp THEN list              { $$ = newflow('I', $2, $4, NULL); }
    | IF exp THEN list ELSE list    { $$ = newflow('I', $2, $4, $6); }
    | WHILE exp DO list             { $$ = newflow('W', $2, $4, NULL); }
    | exp
    ;

list: /* vazio */   { $$ = NULL; }
    | stmt ';' list {
        if ($3 == NULL)
            $$ = $1;
        else
            $$ = newast('L', $1, $3);
    }
    ;

exp: exp CMP exp            { $$ = newcmp($2, $1, $3); }
    | exp '+' exp           { $$ = newast('+', $1, $3); }
    | exp '-' exp           { $$ = newast('-', $1, $3); }
    | exp '*' exp           { $$ = newast('*', $1, $3); }
    | exp '/' exp           { $$ = newast('/', $1, $3); }
    | '|' exp               { $$ = newast('|', $2, NULL); }
    | '(' exp ')'           { $$ = $2; }
    | '-' exp %prec UMINUS  { $$ = newast('M', $2, NULL); }
    | NUMBER                { $$ = newnum($1); }
    | ID                    { $$ = newref($1); }
    | ID '=' exp            { $$ = newasgn($1, $3); }
    | FUNC '(' explist ')'  { $$ = newfunc($1, $3); }
    | ID '(' explist ')'    { $$ = newcall($1, $3); }
    ;

explist: exp
    | exp ',' explist       { $$ = newast('L', $1, $3); }
    ;

symlist: ID                 { $$ = newsymlist($1, NULL); }
    | ID ',' symlist        { $$ = newsymlist($1, $3); }
    ;

calclist: /* comentário ou linha vazia */
    | calclist stmt EOL     {
        fprintf(yyout, "= %4.4g\n", eval($2));
        treefree($2);
    }
    | calclist LET ID '(' symlist ')' '=' list EOL {
        dodef($3, $5, $8);
        fprintf(yyout, "Defined %s\n", $3->name);
    }
    | calclist error EOL {
        yyerrok;
    }
    ;
%%
