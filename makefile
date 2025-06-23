# Nome do compilador C
CC = gcc

# Flags do compilador
# -g: Para debug
# -Wall: Ativa avisos importantes
# -I.: Inclui o diretório raiz do projeto na busca por headers
CFLAGS = -g -Wall -I.

# Flags do linker
# -lpthread: Biblioteca de Pthreads
# -lncurses: Biblioteca Ncurses
LDFLAGS = -lpthread -lncurses

# Nome do executável final
TARGET = icarus_sim

# --- Lista Explícita de Arquivos-Fonte (.c) ---
# Os caminhos agora são relativos à raiz do projeto.
SRCS = \
	Code/S.O/Code/Nucleo/kernel.c \
	Code/S.O/Code/Nucleo/events.c \
	Code/S.O/Code/Process/process.c \
	Code/S.O/Code/escalonador/scheduler.c \
	Code/S.O/Code/Semaforo/semaf.c \
	Code/S.O/Code/Memoria/Paginacao.c \
	Code/S.O/Code/Interface/interface.c \
	Code/S.O/Code/Ferramentas/list.c \
	Code/S.O/Code/Ferramentas/compare.c \
	Code/S.O/Code/IO/io_manager.c \
	Code/S.O/Code/CPU/cpu.c \
	Code/S.O/Code/Clock/clock.c

# Cria os nomes dos arquivos objeto (.o) para serem colocados no diretório 'obj'
# Ex: Code/S.O/Code/Nucleo/kernel.c -> obj/Code/S.O/Code/Nucleo/kernel.o
OBJS := $(patsubst %.c,obj/%.o,$(SRCS))

# --- Regras do Makefile ---

# Regra padrão: construir o executável
all: $(TARGET)

# Regra para linkar o executável final a partir dos objetos
$(TARGET): $(OBJS)
	@echo "==> Linkando o executável final: $(TARGET)..."
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "==> Compilação concluída! Use './$(TARGET)' ou 'make run' para executar."

# Regra de Padrão Estático: Compila cada .c para seu respectivo .o em 'obj/'
$(OBJS): obj/%.o: %.c
	@echo "==> Compilando $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para rodar o programa no terminal atual
run: all
	@echo "==> Executando o simulador no terminal atual..."
	./$(TARGET)

# Regra para limpar os arquivos gerados
clean:
	@echo "==> Limpando arquivos de compilação..."
	-rm -rf obj
	-rm -f $(TARGET)
	@echo "==> Limpeza concluída."

# Declara alvos que não são arquivos
.PHONY: all clean run