# C compiler
CC=gcc
# Compiler flags:
#  -lm		Força o link da biblioteca math.h
#  -Wall	Exibe a maioria dos warnings do compilador
CFLAGS = -lm -Wall

# Parser generator
BISON = bison
# Lexical analyser generator
FLEX = flex

TARGET = fox

$(TARGET): fox-lang.l fox-lang.y fox-lang.h
	$(BISON) -d fox-lang.y
	$(FLEX) -o fox-lang.lex.c fox-lang.l
	$(CC) -o $@ fox-lang.tab.c fox-lang.lex.c fox-lang.c $(CFLAGS)
