#ifndef TRADING_SYSTEM_H
#define TRADING_SYSTEM_H

#include <time.h>

// --- Definições Globais ---
#define MAX_ACOES 10
#define MAX_TRADERS 6
#define MAX_ORDENS 1000
#define MAX_SIMBOLO 10

// --- Tipos de Dados ---
typedef enum {
    COMPRA = 0,
    VENDA = 1
} TipoOrdem;

typedef struct {
    int trader_id;
    char simbolo[MAX_SIMBOLO];
    TipoOrdem tipo;
    double preco;
    int quantidade;
    struct timespec ts_criacao;
    struct timespec ts_execucao;
    char status[20];
} OrdemTrading;

typedef struct {
    char simbolo[MAX_SIMBOLO];
    double preco_atual;
    double preco_abertura;
    int volume_dia;
    double maior_preco;
    double menor_preco;
    int ordens_pendentes;
    time_t ultima_atualizacao;
} AcaoInfo;

typedef struct {
    int total_ordens_processadas;
    double volume_financeiro_total;
    int transacoes_executadas;
    int arbitragens_detectadas;
    int arbitragens_executadas;
    double lucro_potencial_arbitragem;
    double lucro_realizado_arbitragem;
    int inconsistencias_detectadas;
} EstatisticasBolsa;

// --- Protótipos de Funções ---
void inicializar_mercado(AcaoInfo* acoes, EstatisticasBolsa* stats);
int validar_ordem(const OrdemTrading* ordem);
void processar_ordem(OrdemTrading* ordem, AcaoInfo* acoes, EstatisticasBolsa* stats);
void atualizar_preco_acao(AcaoInfo* acao, const OrdemTrading* ordem);
void gerar_ordem(int trader_id, OrdemTrading* ordem, AcaoInfo* acoes);
void monitorar_arbitragem(AcaoInfo* acoes, EstatisticasBolsa* stats);

#endif // TRADING_SYSTEM_H
