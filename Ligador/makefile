# Nome do executável do projeto.

EXE = Ligador

# Nome do compilador, extensão dos arquivos source e dados de compilação
# (flags e bibliotecas).

CC = g++
CFLAGS = -Wall -g
LIBS = -lm

# Nome dos arquivos fontes.

SRC = Ligador.cpp

# Compilação do executável do projeto.

$(EXE): $(SRC)
	$(CC) $(SRC) -o $(EXE) $(CFLAGS) $(LIBS)

# Lista de comandos adicionais do makefile.

.PHONY: clean

# Comando para limpar o executável do projeto.

clean:
	@rm -f *~ core
	@if [ -f $(EXE) ]; then rm $(EXE) -i; fi
