#include <stdio.h>
#include <string.h>
#include <math.h>
#include "trading_system.h"

int validar_ordem(const OrdemTrading* ordem) {
    if (ordem->preco <= 0 || ordem->quantidade <= 0 || ordem->simbolo[0] == '\0') {
        return 0;
    }
    return 1;
}

void processar_ordem(OrdemTrading* ordem, AcaoInfo* acoes, EstatisticasBolsa* stats) {
    if (!validar_ordem(ordem)) {
        strcpy(ordem->status, "REJEITADA");
        return;
    }

    // Procura a ação
    for (int i = 0; i < MAX_ACOES; i++) {
        if (strcmp(acoes[i].simbolo, ordem->simbolo) == 0) {
            // Verifica volatilidade
            double volatilidade = fabs(ordem->preco - acoes[i].preco_atual);
            if (volatilidade > (acoes[i].preco_atual * 0.15)) {
                strcpy(ordem->status, "REJEITADA_VOLATILIDADE");
                return;
            }

            strcpy(ordem->status, "EXECUTADA");
            stats->transacoes_executadas++;
            stats->volume_financeiro_total += ordem->preco * ordem->quantidade;
            
            // Atualiza preço da ação
            atualizar_preco_acao(&acoes[i], ordem);
            return;
        }
    }

    strcpy(ordem->status, "REJEITADA_SIMBOLO_INVALIDO");
}
