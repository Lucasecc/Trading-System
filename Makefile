CC=gcc
CFLAGS=-Wall -pthread
SRC=codigo
BIN=bin

PROCESSOS_OBJS=$(SRC)/main_processos.c $(SRC)/trader.c $(SRC)/executor.c $(SRC)/price_updater.c $(SRC)/arbitrage_monitor.c
THREADS_OBJS=$(SRC)/main_threads.c $(SRC)/trader.c $(SRC)/executor.c $(SRC)/price_updater.c $(SRC)/arbitrage_monitor.c

all: processos threads

$(BIN):
	mkdir -p $(BIN)

processos: $(BIN) $(PROCESSOS_OBJS)
	$(CC) $(CFLAGS) -o $(BIN)/processos_exec $(PROCESSOS_OBJS)

threads: $(BIN) $(THREADS_OBJS)
	$(CC) $(CFLAGS) -o $(BIN)/threads_exec $(THREADS_OBJS)

test: all
	@echo "=== TESTE VERSÃO PROCESSOS ==="
	./$(BIN)/processos_exec 4 2 10
	@echo ""
	@echo "=== TESTE VERSÃO THREADS ==="
	./$(BIN)/threads_exec 4 2 10

clean:
	rm -rf $(BIN)
	rm -f *.o

.PHONY: all clean processos threads test
