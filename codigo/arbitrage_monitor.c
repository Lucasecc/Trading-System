#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "trading_system.h"

void monitorar_arbitragem(AcaoInfo* acoes, EstatisticasBolsa* stats) {
    for (int i = 0; i < MAX_ACOES; i++) {
        if (acoes[i].simbolo[0] == '\0') continue;

        double variacao_percentual = ((acoes[i].preco_atual - acoes[i].preco_abertura) / acoes[i].preco_abertura) * 100.0;
        
        if (fabs(variacao_percentual) > 5.0) {
            stats->arbitragens_detectadas++;
            stats->lucro_potencial_arbitragem += fabs(variacao_percentual) * 100.0;
        }
    }
}
