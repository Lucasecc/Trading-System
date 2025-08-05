#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "trading_system.h"

void gerar_ordem(int trader_id, OrdemTrading* ordem, AcaoInfo* acoes) {
    // Escolhe uma ação aleatória que existe
    int idx;
    do {
        idx = rand() % MAX_ACOES;
    } while (acoes[idx].simbolo[0] == '\0');
    
    strcpy(ordem->simbolo, acoes[idx].simbolo);
    ordem->tipo = rand() % 2;
    ordem->preco = acoes[idx].preco_atual * (0.95 + (rand() % 100) / 1000.0); // ±5% do preço atual
    ordem->quantidade = 100 + (rand() % 901); // 100-1000
    ordem->trader_id = trader_id;
    strcpy(ordem->status, "PENDENTE");
    clock_gettime(CLOCK_MONOTONIC, &ordem->ts_criacao);
}
