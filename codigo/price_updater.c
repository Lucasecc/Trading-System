#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "trading_system.h"

void inicializar_mercado(AcaoInfo* acoes, EstatisticasBolsa* stats) {
    const char* simbolos[] = {
        "PETR4", "VALE3", "ITUB4", "ABEV3", "BBAS3", "MGLU3", "WEGE3", "B3SA3"
    };
    const double precos_iniciais[] = {
        25.50, 45.20, 28.90, 14.80, 37.10, 4.95, 36.80, 11.75
    };
    
    for (int i = 0; i < MAX_ACOES; i++) {
        acoes[i].simbolo[0] = '\0';
    }

    for (int i = 0; i < 8; i++) {
        strcpy(acoes[i].simbolo, simbolos[i]);
        acoes[i].preco_abertura = precos_iniciais[i];
        acoes[i].preco_atual = precos_iniciais[i];
        acoes[i].volume_dia = 0;
        acoes[i].ordens_pendentes = 0;
        acoes[i].maior_preco = precos_iniciais[i];
        acoes[i].menor_preco = precos_iniciais[i];
        acoes[i].ultima_atualizacao = time(NULL);
    }

    stats->total_ordens_processadas = 0;
    stats->volume_financeiro_total = 0.0;
    stats->transacoes_executadas = 0;
    stats->arbitragens_detectadas = 0;
    stats->arbitragens_executadas = 0;
    stats->lucro_potencial_arbitragem = 0.0;
    stats->lucro_realizado_arbitragem = 0.0;
    stats->inconsistencias_detectadas = 0;
}

void atualizar_preco_acao(AcaoInfo* acao, const OrdemTrading* ordem) {
    double impacto = (ordem->tipo == COMPRA) ? 0.01 : -0.01;
    double novo_preco = acao->preco_atual + (impacto * (ordem->quantidade / 100.0));
    
    if (novo_preco > 0) {
        acao->preco_atual = novo_preco;
    }
    
    acao->volume_dia += ordem->quantidade;
    acao->ultima_atualizacao = time(NULL);
    
    if (acao->preco_atual > acao->maior_preco) {
        acao->maior_preco = acao->preco_atual;
    }
    if (acao->preco_atual < acao->menor_preco) {
        acao->menor_preco = acao->preco_atual;
    }
}
