# CCO007-fox-lang
Compilador desenvolvido com propósitos educacionais.


DEPENDÊNCIAS
- Make
- Bison
- Flex
- GCC


COMPILAÇÃO DE UM CÓDIGO FOX
- No terminal, execute o comando
    "make -B"
  * Este comando só precisa ser executado uma vez.

- Execute o comando
    "./fox <nome_arquivo>.fox"  

Após a compilação, um código em C, nomeado out.c, é gerado.


COMPILAÇÃO E EXECUÇÃO DO CÓDIGO C GERADO
- Compile o código C
    "gcc -o <nome_executavel> out.c -Wall"

- Execute o arquivo
    "./<nome_executavel>"
