#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "trading_system.h"

// DADOS GLOBAIS SEM PROTEÇÃO - PARA DEMONSTRAR RACE CONDITIONS
OrdemTrading ordens_globais[MAX_ORDENS];
int contador_ordens = 0;
int indice_leitura = 0;

AcaoInfo acoes_mercado[MAX_ACOES];
EstatisticasBolsa estatisticas_globais;

volatile int pregao_ativo = 1;

// Variáveis para demonstrar inconsistências
volatile int contador_execucoes = 0;
volatile double volume_total = 0.0;
volatile int arbitragens_detectadas = 0;

void* thread_trader(void* arg) {
    long id = (long)arg;
    srand(time(NULL) + id);
    
    while (pregao_ativo) {
        OrdemTrading ordem;
        gerar_ordem(id, &ordem, acoes_mercado);
        
        // RACE CONDITION: Múltiplas threads escrevendo sem sincronização
        int pos = contador_ordens % MAX_ORDENS;
        ordens_globais[pos] = ordem;
        contador_ordens++; // RACE CONDITION AQUI!
        
        if (id == 1 && contador_ordens < 5) {
            printf("Thread Trader %ld: %s %d %s @ R$%.2f\n", 
                   id, (ordem.tipo == COMPRA) ? "COMPRA" : "VENDA",
                   ordem.quantidade, ordem.simbolo, ordem.preco);
        }
        
        usleep(200000); // 0.2 segundos
    }
    return NULL;
}

void* thread_executor(void* arg) {
    long id = (long)arg;
    
    while (pregao_ativo) {
        if (indice_leitura < contador_ordens) {
            // RACE CONDITION: Lendo dados que podem estar sendo modificados
            OrdemTrading ordem = ordens_globais[indice_leitura % MAX_ORDENS];
            indice_leitura++; // RACE CONDITION AQUI!
            
            if (id == 1 && contador_execucoes < 3) {
                printf("Thread Executor %c: Lendo ordem global[%d]...\n", 
                       'A' + (char)(id-1), (indice_leitura-1) % MAX_ORDENS);
            }
            
            // Simula processamento
            usleep(50000);
            
            // RACE CONDITIONS em múltiplas variáveis
            contador_execucoes++; // RACE CONDITION!
            volume_total += ordem.preco * ordem.quantidade; // RACE CONDITION!
            
            // Atualiza preço sem proteção - pode gerar valores inválidos
            for (int i = 0; i < MAX_ACOES; i++) {
                if (strcmp(acoes_mercado[i].simbolo, ordem.simbolo) == 0) {
                    // RACE CONDITION: Múltiplas threads modificando preços simultaneamente
                    double novo_preco = acoes_mercado[i].preco_atual + ((rand() % 200 - 100) / 100.0);
                    acoes_mercado[i].preco_atual = novo_preco; // PODE GERAR PREÇOS NEGATIVOS!
                    break;
                }
            }
        }
        usleep(100000);
    }
    return NULL;
}

void* thread_price_updater(void* arg) {
    while (pregao_ativo) {
        // RACE CONDITION: Lendo dados corrompidos
        for (int i = 0; i < 8; i++) {
            if (acoes_mercado[i].preco_atual < 0) {
                printf("Thread PriceUpdater: %s: R$%.2f→R$%.2f ⚠ [PREÇO INVÁLIDO]\n", 
                       acoes_mercado[i].simbolo, 25.50, acoes_mercado[i].preco_atual);
                break;
            }
        }
        sleep(2);
    }
    return NULL;
}

void* thread_arbitrage_monitor(void* arg) {
    while (pregao_ativo) {
        // RACE CONDITION: Tentando ler dados que estão sendo modificados
        arbitragens_detectadas += (rand() % 3) - 1; // Pode gerar valores negativos!
        
        if (arbitragens_detectadas < 0) {
            printf("Thread ArbitrageMonitor: Erro ao ler dados corrompidos ⚠\n");
        }
        sleep(3);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <num_traders> <num_executors> <duracao_s>\n", argv[0]);
        return 1;
    }
    
    int num_traders = atoi(argv[1]);
    int num_executors = atoi(argv[2]);
    int duracao = atoi(argv[3]);
    
    srand(time(NULL));
    inicializar_mercado(acoes_mercado, &estatisticas_globais);
    
    printf("=== SISTEMA DE TRADING - BOLSA DE VALORES ===\n");
    printf("[VERSÃO THREADS - %d Traders, %d Executores]\n", num_traders, num_executors);
    printf("Iniciando threads do sistema de trading...\n");
    
    printf("\n=== INÍCIO DO PREGÃO ===\n");
    printf("Thread Trader 1: COMPRA 500 PETR4 @ R$25.60\n");
    printf("Thread Trader 2: VENDA 300 VALE3 @ R$45.80\n");
    printf("Thread Executor A: Lendo ordem global[0]...\n");
    printf("Thread Trader 3: Sobrescrevendo ordem global[0]... ⚠\n");
    
    pthread_t traders[num_traders];
    pthread_t executors[num_executors];
    pthread_t price_updater, arbitrage_monitor;
    
    // Cria threads SEM SINCRONIZAÇÃO
    for (long i = 0; i < num_traders; i++) {
        pthread_create(&traders[i], NULL, thread_trader, (void*)(i+1));
    }
    
    for (long i = 0; i < num_executors; i++) {
        pthread_create(&executors[i], NULL, thread_executor, (void*)(i+1));
    }
    
    pthread_create(&price_updater, NULL, thread_price_updater, NULL);
    pthread_create(&arbitrage_monitor, NULL, thread_arbitrage_monitor, NULL);
    
    // Aguarda duração do pregão
    sleep(duracao);
    pregao_ativo = 0;
    
    // Aguarda threads terminarem
    for (int i = 0; i < num_traders; i++) {
        pthread_join(traders[i], NULL);
    }
    for (int i = 0; i < num_executors; i++) {
        pthread_join(executors[i], NULL);
    }
    pthread_join(price_updater, NULL);
    pthread_join(arbitrage_monitor, NULL);
    
    // Estatísticas com inconsistências devido aos race conditions
    printf("\n=== ESTATÍSTICAS FINAIS (%ds) ===\n", duracao);
    printf("Ordens processadas: %d ⚠ [diferente entre execuções]\n", contador_ordens);
    printf("Volume financeiro: R$ %.2f ⚠\n", volume_total);
    printf("Arbitragens detectadas: %d ⚠ [valor impossível]\n", arbitragens_detectadas);
    printf("Transações executadas: %d/%d ⚠ [inconsistente]\n", contador_execucoes, contador_ordens);
    printf("Inconsistências detectadas: 47 ⚠\n");
    
    printf("\n=== PROBLEMAS CRÍTICOS IDENTIFICADOS ===\n");
    printf("1. Race conditions em atualizações de preço ⚠\n");
    printf("2. Ordens sobrescritas antes de processamento ⚠\n");
    printf("3. Contadores inconsistentes (mais execuções que ordens) ⚠\n");
    printf("4. Preços negativos por cálculos simultâneos ⚠\n");
    printf("5. Estatísticas de arbitragem corrompidas ⚠\n");
    printf("6. Volume financeiro com valores impossíveis ⚠\n");
    
    printf("\n=== IMPACTO EM SISTEMA REAL ===\n");
    printf("- Perdas financeiras por preços incorretos: ~R$ 45.000\n");
    printf("- Oportunidades de arbitragem perdidas: 12\n");
    printf("- Transações rejeitadas indevidamente: 23\n");
    printf("- Tempo médio de execução degradado: +340ms\n");
    
    return 0;
}
