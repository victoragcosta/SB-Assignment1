# Nome do executável do projeto.

EXE = Ligador

# Nome do compilador, extensão dos arquivos source e dados de compilação
# (flags e bibliotecas).

CC = g++
EXT = .cpp
CFLAGS = -Wall -g -I $(IDIR)
LIBS = -lm

# Caminhos até pastas importantes (arquivos src, arquivos .h e arquivos .o).

IDIR = include
ODIR = src/obj
SDIR = src

# Lista de dependências do projeto (arquivos .h).

_DEPS =

# Lista de arquivos intermediários de compilação gerados pelo projeto
# (arquivos .o).

_OBJ = Ligador.o

# Lista de arquivos fontes utilizados para compilação.

_SRC = Ligador.cpp

# Junção dos nomes de arquivos com seus respectivos caminhos.

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
SRC = $(patsubst %,$(SDIR)/%,$(_SRC))

# Atualização de arquivos que foram alterados.

$(ODIR)/%.o: $(SDIR)/%$(EXT) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Compilação do executável do projeto.

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

# Lista de comandos adicionais do makefile.

.PHONY: clean
.PHONY: structure
.PHONY: verification

# Comando para limpar o executável do projeto e os arquivos .o.

clean:
	@rm -f $(ODIR)/*.o *~ core
	@if [ -f $(EXE) ]; then rm $(EXE) -i; fi

# Comando para gerar a estrutura inicial do projeto.

structure:

	# Criação das pastas do projeto.

	mkdir include
	mkdir src
	mkdir src/obj

	# Movimentação dos arquivos existentes para suas respectivas pastas.

	if [ -f *.h ]; then mv *.h $(IDIR); fi
	if [ -f *$(EXT) ]; then mv *$(EXT) $(SDIR); fi
	if [ -f *.o ]; then mv *.o $(ODIR); fi

# Comando para verificar os testes utilizando o cppcheck e o valgrind.

verification:
	cppcheck $(SRC) ./$(EXE) --enable=all
	valgrind --leak-check=full ./$(EXE)
