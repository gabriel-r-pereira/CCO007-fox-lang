/**
 *  Declarações usadas pelo lexer da linguagem
 */

/* Interface para o lexer */
extern int yylineno;        // armazena o número da linha
extern FILE *yyin;
extern FILE *yyout;
void yyerror(char *s, ...);

/* Estrutura usada para representar um símbolo, na tabela de símbolos */
struct symbol {
    char *name;             // nome da variável
    double value;           // valor da variável
    struct ast *func;       // AST que representa as declarações de uma função
    struct symlist *syms;   // lista de argumentos da função
};

#define NHASH 9997  // tamanho da tabela de símbolos
/* Tabela de símbolos */
struct symbol symtab[NHASH];

/* Função para busca na tabela de símbolos */
struct symbol *lookup(char*);

/* Lista de símbolos, usado na passagem de argumentos */
struct symlist {
    struct symbol *sym;
    struct symlist *next;
};

/* Cria lista de símbolos */
struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
/* Limpa uma lista de símbolos */
void symlistfree(struct symlist *sl);


/*
 * Opções para nodetype
 * + - * / |
 * 0-7  operações de comparação
 * M    menos unário
 * L    expressão ou lista de declarações (statements)
 * I    IF
 * W    WHILE
 * N    símbolo
 * =    atribuição
 * S    lista de símbolos
 * F    função da linguagem
 * C    função do usuário
*/

/* Funções da linguagem */
enum bifs {
    B_sqrt = 1,
    B_exp,
    B_log,
    B_print,
};

/* Nós da Abstract Syntax Tree (AST) */
/* Cada tipo de nó possui um nodetype padrão */
/* Nó usado para expressões e comparação */
struct ast {
    int nodetype;       // tipo L ou 0-7
    struct ast *l;      // nó da esquerda
    struct ast *r;      // nó da direita
};

/* Nó de função da linguagem */
struct fncall {
    int nodetype;       // tipo F
    struct ast *l;      // expressão passada como parâmetro
    enum bifs functype;
};

/* Nó de função do usuário */
struct ufncall {
    int nodetype;       // tipo C
    struct ast *l;      // AST da função
    struct symbol *s;   // símbolo da função
};

/* Nó de estruturas de controle */
struct flow {
    int nodetype;       // tipo I ou W
    struct ast *cond;   // condição
    struct ast *tl;     // se condição - lista de declarações
    struct ast *el;     // senão (opcional) - declarações
};

/* Nó de número */
struct numval {
    int nodetype;       // tipo K
    double number;      // valor numérico
};

/* Nó de símbolo */
struct symref {
    int nodetype;       // tipo N
    struct symbol *s;   // símbolo
};

/* Nó de atribuição */
struct symasgn {
    int nodetype;       // tipo =
    struct symbol *s;   // símbolo
    struct ast *v;      // valor
};


/* Cia um nó da AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
/* Cia um nó de comparação */
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
/* Cia um nó de função da linguagem */
struct ast *newfunc(int functype, struct ast *l);
/* Cia um nó de função do usuário */
struct ast *newcall(struct symbol *s, struct ast *l);
/* Cia um nó de símbolo */
struct ast *newref(struct symbol *s);
/* Cia um nó de atribuição */
struct ast *newasgn(struct symbol *s, struct ast *v);
/* Cria um nó de número*/
struct ast *newnum(double d);
/* Cia um nó de estrutura de controle */
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

/* Define uma função */
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/* Avalia a expressão representada por uma AST */
double eval(struct ast *);
/* Limpa uma AST */
void treefree(struct ast *);
