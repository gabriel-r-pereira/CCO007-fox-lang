/*
 *  Funções auxiliares
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "parser.h"

static unsigned symhash(char *sym) {
    unsigned int hash = 0;
    unsigned c;

    while ( (c = *sym++) ) hash = hash * 9 ^ c;

    return hash;
}

struct symbol *getsymbol(char *sym) {
    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH;

    while (--scount >= 0) {
        if (sp->name && !strcmp(sp->name, sym)) {
            return sp;
        }

        if (++sp >= symtab + NHASH) // tenta o próximo símbolo
            sp = symtab;
    }

    return NULL;
}

struct symbol *newsymbol(int type, char *sym) {
    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH;

    while (--scount >= 0) {
        if (sp->name && !strcmp(sp->name, sym)) {
            yyerror("symbol '%s' already declared.", sp->name);
            abort();
        }

        // adiciona novo símbolo
        if (!sp->name) {
            sp->name = strdup(sym);
            sp->type = type;
            sp->value = 0;
            sp->func = NULL;
            sp->syms = NULL;
            return sp;
        }

        if (++sp >= symtab + NHASH) // tenta o próximo símbolo
            sp = symtab;
    }

    yyerror("symbol table overflow");
    abort();    // a tabela está cheia
}

struct ast *newast(int nodetype, struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newint(int i) {
    struct intval *a = malloc(sizeof(struct intval));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = 'X';
    a->number = i;
    return (struct ast *)a;
}

struct ast *newfloat(double f) {
    struct floatval *a = malloc(sizeof(struct floatval));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = 'Y';
    a->number = f;
    return (struct ast *)a;
}

struct ast *newchar(char c) {
    struct charval *a = malloc(sizeof(struct charval));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = 'Z';
    a->c = c;
    return (struct ast *)a;
}

struct ast *newcmp(int cmptype, struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newlgi(int logtype, struct ast *l, struct ast *r){
	struct ast *a = malloc(sizeof(struct ast));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = '0' + logtype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newfunc(int functype, struct ast *l) {
    struct fncall *a = malloc(sizeof(struct fncall));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = 'F';
    a->l = l;
    a->functype = functype;
    return (struct ast *)a;
}

struct ast *newref(struct symbol *s) {
    struct symref *a = malloc(sizeof(struct symref));
    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = 'N';
    a->s = s;
    return (struct ast *)a;
}

struct ast *newasgn(struct symbol *s, struct ast *v) {
    struct symasgn *a = malloc(sizeof(struct symasgn));
    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = '=';
    a->s = s;
    a->v = v;
    return (struct ast *)a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el) {
    struct flow *a = malloc(sizeof(struct flow));

    if(!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;
    return (struct ast *)a;
}

void treefree(struct ast *a) {
    switch(a->nodetype) {
        /* possuem duas sub-árvores (esquerda e direita) */
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
        case 'L':
            treefree(a->r);
        /* possuem uma sub-árvore (esquerda) */
        case 'M': case '!': case 'F':
            treefree(a->l);
        /* não possuem sub-árvores */
        case 'X': case 'Y': case 'Z': case 'N':
            break;
        case '=':
            treefree(((struct symasgn *)a)->v);
            break;
        /* possuem até três subárvores */
        case 'I': case 'W':
            treefree( ((struct flow *)a)->cond);
            if(((struct flow *)a)->tl)
                treefree(((struct flow *)a)->tl);
            if(((struct flow *)a)->el)
                treefree(((struct flow *)a)->el);
            break;
        default:
            printf("internal error: free bad node %c\n", a->nodetype);
    }

    free(a); // O próprio nó é sempre limpo
}

struct idlist *newidlist(char *id, struct idlist *next) {
    struct idlist *il = malloc(sizeof(struct idlist));
    if(!il) {
        yyerror("out of space");
        exit(0);
    }

    il->id = strdup(id);
    il->next = next;
    return il;
}

static void callbuiltin(struct fncall *f, char *prepend) {
    enum bifs functype = f->functype;

    fprintf(output, "%s", prepend);
    switch(functype) {
        case B_sqrt:
            fprintf(output, "sqrt(");
            eval(f->l, "");
            fprintf(output, ");\n");
            break;
        case B_exp:
            fprintf(output, "exp(");
            eval(f->l, "");
            fprintf(output, ");\n");
            break;
        case B_log:
            fprintf(output, "log(");
            eval(f->l, "");
            fprintf(output, ");\n");
            break;
        case B_print:
            switch (f->l->nodetype) {
                case 'X':
                    fprintf(output, "printf(\"%%d\\n\", ");
                    eval(f->l, "");
                    fprintf(output, ");\n");
                    break;
                case 'Y':
                    fprintf(output, "printf(\"%%lf\\n\", ");
                    eval(f->l, "");
                    fprintf(output, ");\n");
                    break;
                case 'Z':
                    fprintf(output, "printf(\"%%c\\n\", '");
                    eval(f->l, "");
                    fprintf(output, "');\n");
                    break;
                case 'N':
                    switch (((struct symref *)f->l)->s->type) {
                        case T_int:
                            fprintf(output, "printf(\"%%d\\n\", ");
                            eval(f->l, "");
                            fprintf(output, ");\n");
                            break;
                        case T_float:
                            fprintf(output, "printf(\"%%lf\\n\", ");
                            eval(f->l, "");
                            fprintf(output, ");\n");
                            break;
                        case T_char:
                            fprintf(output, "printf(\"%%c\\n\", ");
                            eval(f->l, "");
                            fprintf(output, ");\n");
                            break;
                        default:
                            yyerror("scan: argument 1 has invalid type");
                            exit(0);
                    }
                    break;
                default:
                    yyerror("print: invalid argument 1");
                    exit(0);
            }
            break;
        case B_scan:
            if (f->l->nodetype != 'N') {
                yyerror("scan: expected a variable argument");
                exit(0);
            } else {
                switch (((struct symref *)f->l)->s->type) {
                    case T_int:
                        fprintf(output, "scanf(\"%%d\", &");
                        eval(f->l, "");
                        fprintf(output, ");\n");
                        break;
                    case T_float:
                        fprintf(output, "scanf(\"%%lf\", &");
                        eval(f->l, "");
                        fprintf(output, ");\n");
                        break;
                    case T_char:
                        fprintf(output, "scanf(\" %%c\", &");
                        eval(f->l, "");
                        fprintf(output, ");\n");
                        break;
                    default:
                        yyerror("scan: argument 1 has invalid type");
                        exit(0);
                }
            }
            break;
        default:
            yyerror("Unknown built-in function %d", functype);
    }
}

void eval(struct ast *a, char *prepend) {
    char *addprep;
    if(!a) {
        yyerror("internal error, null eval");
    }
    if (a->nodetype != 'L')
        fprintf(output, "%s", prepend);

    switch(a->nodetype) {
        /* Número inteiro */
        case 'X':
            fprintf(output, "%d", ((struct intval *)a)->number);
            fprintf(commands, "%5d:\tINT LITERAL\n", yylineno);
            break;
        /* ponto flutuante */
        case 'Y':
            fprintf(output, "%lf", ((struct floatval *)a)->number);
            fprintf(commands, "%5d:\tFLOAT LITERAL\n", yylineno);
            break;
        /* char */
        case 'Z':
            fprintf(output, "%c", ((struct charval *)a)->c);
            fprintf(commands, "%5d:\tCHAR LITERAL\n", yylineno);
            break;
        /* Identificador */
        case 'N':
            fprintf(output, "%s", ((struct symref *)a)->s->name);
            switch (((struct symref *)a)->s->type) {
                case T_int:
                    fprintf(commands, "%5d:\tINT", yylineno);
                    break;
                case T_float:
                    fprintf(commands, "%5d:\tFLOAT", yylineno);
                    break;
                case T_char:
                    fprintf(commands, "%5d:\tCHAR", yylineno);
                    break;
            }
            fprintf(commands, " PALAVRA RESERVADA\n");
            break;
        /* Atribuição */
        case '=':
            fprintf(output, "%s = ", ((struct symasgn *)a)->s->name);
            eval(((struct symasgn *)a)->v, "");
            fprintf(output, ";\n");
            fprintf(commands, "%5d:\tOPERADOR DE ATRIBUIÇÃO\n", yylineno);
            break;
        /* Expressões */
        case '+':
            fprintf(output, " + ");
            eval(a->l, "");
            fprintf(output, " + ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR DE SOMA\n", yylineno);
            break;
        case '-':
            eval(a->l, "");
            fprintf(output, " - ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR DE SUBTRAÇÃO\n", yylineno);
            break;
        case '*':
            eval(a->l, "");
            fprintf(output, " * ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR DE MULTIPLICAÇÃO\n", yylineno);
            break;
        case '/':
            eval(a->l, "");
            fprintf(output, " / ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR DE DIVISÃO\n", yylineno);
            break;
        case '%':
            eval(a->l, "");
            fprintf(output, " %% ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR MOD\n", yylineno);
            break;
        case 'M':
            fprintf(output, "-");
            eval(a->l, "");
            fprintf(commands, "%5d:\tMENOS UNÁRIO\n", yylineno);
            break;
        case '!':
            fprintf(output, "!");
            eval(a->l, "");
            fprintf(commands, "%5d:\tOPERADOR DE NEGAÇÃO\n", yylineno);
            break;
        /* Comparações */
        case '1':
            eval(a->l, "");
            fprintf(output, " > ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR MAIOR COMPARAÇÃO\n", yylineno);
            break;
        case '2':
            eval(a->l, "");
            fprintf(output, " < ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR MENOR COMPARAÇÃO\n", yylineno);
            break;
        case '3':
            eval(a->l, "");
            fprintf(output, " != ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR DIFERENTE COMPARAÇÃO\n", yylineno);
            break;
        case '4':
            eval(a->l, "");
            fprintf(output, " == ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR IGUAL COMPARAÇÃO\n", yylineno);
            break;
        case '5':
            eval(a->l, "");
            fprintf(output, " >= ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR MAIOR IGUAL COMPARAÇÃO\n", yylineno);
            break;
        case '6':
            eval(a->l, "");
            fprintf(output, " <= ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR MENOR IGUAL COMPARAÇÃO\n", yylineno);
            break;
        case '7':
            eval(a->l, "");
            fprintf(output, " && ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR E LÓGICO\n", yylineno);
            break;
        case '8':
            eval(a->l, "");
            fprintf(output, " || ");
            eval(a->r, "");
            fprintf(commands, "%5d:\tOPERADOR OU LÓGICO\n", yylineno);
            break;
        /* Estruturas de controle */
        /* Expressões nulas são permitidas, portanto são checadas */
        /* if/then/else */
        case 'I':
            fprintf(output, "\n%s", prepend);
            fprintf (output, "if (");
            eval(((struct flow *)a)->cond, "");
            fprintf(output, ") {\n");

            if(((struct flow *)a)->tl) {
                addprep = strdup(prepend);
                strcat(addprep, "\t");

                eval(((struct flow *)a)->tl, addprep);
            }

            fprintf(output, "%s", prepend);
            fprintf(output, "}");

            if(((struct flow *)a)->el) {
                addprep = strdup(prepend);
                strcat(addprep, "\t");

                fprintf (output, " else {\n");
                eval(((struct flow *)a)->el, addprep);

                fprintf(output, "%s", prepend);
                fprintf(output, "}\n");
            }
            fprintf(commands, "%5d:\tCOMANDO CONDICIONAL\n", yylineno);
            break;
        case 'W':
            fprintf(output, "\n%s", prepend);
            fprintf(output, "while (");
            eval(((struct flow *)a)->cond, prepend);
            fprintf(output, ") {\n");

            if(((struct flow *)a)->tl) {
                addprep = strdup(prepend);
                strcat(addprep, "\t");

                eval(((struct flow *)a)->tl, addprep);
            }

            fprintf(output, "%s", prepend);
            fprintf(output, "}\n\n");
            fprintf(commands, "%5d:\tCOMANDO DE REPETIÇÃO\n", yylineno);
            break;
        /* Lista de declarações */
        case 'L':
            eval(a->l, prepend);
            eval(a->r, prepend);
            break;
        case 'F':
            callbuiltin((struct fncall *)a, "");
            fprintf(commands, "%5d:\tCHAMADA DE FUNÇÃO\n", yylineno);
            break;
        default:
            printf("internal error: bad node %c\n", a->nodetype);
    }
}

void defvar(int type, struct idlist *syms, char *prepend) {
    struct idlist *til;

    fprintf(output, "%s", prepend);
    switch (type) {
        case T_int:
            fprintf(output, "int ");
            break;
        case T_float:
            fprintf(output, "double ");
            break;
        case T_char:
            fprintf(output, "char ");
            break;
    }

    while (syms) {
        newsymbol(type, syms->id);
        til = syms->next;

        if (til) {
            fprintf(output, "%s, ", syms->id);
        } else {
            fprintf(output, "%s;\n", syms->id);
        }

        free(syms);
        syms = til;
    }
    fprintf(commands, "%5d:\tDECLARAÇÃO DE VARIAVEIS\n", yylineno);
}

void closeoutputfiles() {
    fprintf(output, "\n\treturn 0;");
    fprintf(output, "\n}\n");
    fclose(output);

    fclose(commands);
}

void yyerror(char *s, ...) {
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

int main (int argc, char **argv) {
    if (argc >= 2) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    } else {
        printf("error: input file not provided\n");
        exit(0);
    }
    if (!(output = fopen("out.c", "w"))) {
        perror("out.c");
        return 1;
    }
    if (!(commands = fopen("commands.txt", "w"))) {
        perror("commands.txt");
        return 1;
    }

    fprintf(output, "#include <stdio.h>\n");
    fprintf(output, "#include <math.h>\n");
    fprintf(output, "#include <stdlib.h>\n\n");
    fprintf(output, "int main(int argc, char **argv) {\n");

    fprintf(commands, "LINHA\tSÍMBOLO\n");

    return yyparse();
}
