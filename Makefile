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

# Nome do executável
TARGET = fox

$(TARGET): scanner.l parser.y parser.h
	$(BISON) -d parser.y
	$(FLEX) -o scanner.lex.c scanner.l
	$(CC) -o $@ parser.tab.c scanner.lex.c parser.c $(CFLAGS)
