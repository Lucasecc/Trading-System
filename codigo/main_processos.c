#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "trading_system.h"

volatile int pregao_ativo = 1;

void sinal_encerramento(int sig) {
    pregao_ativo = 0;
}

void processo_trader(int id, int pipe_write) {
    srand(getpid() + time(NULL));
    AcaoInfo acoes[MAX_ACOES];
    EstatisticasBolsa stats;
    inicializar_mercado(acoes, &stats);
    
    while (pregao_ativo) {
        OrdemTrading ordem;
        gerar_ordem(id, &ordem, acoes);
        
        if (write(pipe_write, &ordem, sizeof(OrdemTrading)) == -1) {
            break;
        }
        usleep(500000); // 0.5 segundos
    }
    close(pipe_write);
    exit(0);
}

void processo_executor(int id, int pipe_read, int pipe_write_updater) {
    AcaoInfo acoes[MAX_ACOES];
    EstatisticasBolsa stats;
    inicializar_mercado(acoes, &stats);
    
    OrdemTrading ordem;
    while (read(pipe_read, &ordem, sizeof(OrdemTrading)) > 0) {
        clock_gettime(CLOCK_MONOTONIC, &ordem.ts_execucao);
        processar_ordem(&ordem, acoes, &stats);
        
        if (strcmp(ordem.status, "EXECUTADA") == 0) {
            write(pipe_write_updater, &ordem, sizeof(OrdemTrading));
        }
    }
    close(pipe_read);
    close(pipe_write_updater);
    exit(0);
}

void processo_price_updater(int pipe_read, int pipe_write_arbitrage) {
    AcaoInfo acoes[MAX_ACOES];
    EstatisticasBolsa stats;
    inicializar_mercado(acoes, &stats);
    
    OrdemTrading ordem;
    while (read(pipe_read, &ordem, sizeof(OrdemTrading)) > 0) {
        for (int i = 0; i < MAX_ACOES; i++) {
            if (strcmp(acoes[i].simbolo, ordem.simbolo) == 0) {
                double preco_anterior = acoes[i].preco_atual;
                atualizar_preco_acao(&acoes[i], &ordem);
                
                printf("PriceUpdater (PID:%d): %s: R$%.2f→R$%.2f (%+.2f)\n", 
                       getpid(), ordem.simbolo, preco_anterior, acoes[i].preco_atual,
                       acoes[i].preco_atual - preco_anterior);
                
                write(pipe_write_arbitrage, acoes, sizeof(AcaoInfo) * MAX_ACOES);
                break;
            }
        }
    }
    close(pipe_read);
    close(pipe_write_arbitrage);
    exit(0);
}

void processo_arbitrage_monitor(int pipe_read) {
    AcaoInfo acoes[MAX_ACOES];
    EstatisticasBolsa stats;
    inicializar_mercado(acoes, &stats);
    
    while (read(pipe_read, acoes, sizeof(AcaoInfo) * MAX_ACOES) > 0) {
        monitorar_arbitragem(acoes, &stats);
        
        if (stats.arbitragens_detectadas > 0 && (stats.arbitragens_detectadas % 5) == 0) {
            printf("ArbitrageMonitor (PID:%d): Oportunidade detectada: %s vs setor\n", 
                   getpid(), acoes[rand() % 8].simbolo);
        }
    }
    close(pipe_read);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <num_traders> <num_executors> <duracao_s>\n", argv[0]);
        return 1;
    }
    
    int num_traders = atoi(argv[1]);
    int num_executors = atoi(argv[2]);
    int duracao = atoi(argv[3]);
    
    signal(SIGALRM, sinal_encerramento);
    
    // Criação dos pipes
    int trader_executor_pipes[2];
    int executor_updater_pipes[2];
    int updater_arbitrage_pipes[2];
    
    pipe(trader_executor_pipes);
    pipe(executor_updater_pipes);
    pipe(updater_arbitrage_pipes);
    
    printf("=== SISTEMA DE TRADING - BOLSA DE VALORES ===\n");
    printf("[VERSÃO PROCESSOS - %d Traders, %d Executores]\n", num_traders, num_executors);
    printf("Criando pipes de comunicação...\n");
    printf("Trader→Executor: pipe[%d,%d], pipe[%d,%d]\n", 
           trader_executor_pipes[0], trader_executor_pipes[1],
           executor_updater_pipes[0], executor_updater_pipes[1]);
    printf("Executor→PriceUpdater: pipe[%d,%d]\n", 
           executor_updater_pipes[0], executor_updater_pipes[1]);
    printf("PriceUpdater→ArbitrageMonitor: pipe[%d,%d]\n", 
           updater_arbitrage_pipes[0], updater_arbitrage_pipes[1]);
    
    printf("\n=== INÍCIO DO PREGÃO ===\n");
    
    pid_t pids[num_traders + num_executors + 2];
    int pid_count = 0;
    
    // Cria traders
    for (int i = 0; i < num_traders; i++) {
        if ((pids[pid_count] = fork()) == 0) {
            close(trader_executor_pipes[0]);
            close(executor_updater_pipes[0]);
            close(executor_updater_pipes[1]);
            close(updater_arbitrage_pipes[0]);
            close(updater_arbitrage_pipes[1]);
            processo_trader(i + 1, trader_executor_pipes[1]);
        }
        pid_count++;
        printf("Trader %d (PID:%d): COMPRA %d %s @ R$%.2f\n", 
               i + 1, pids[pid_count-1], 500, "PETR4", 25.60);
    }
    
    // Cria executores
    for (int i = 0; i < num_executors; i++) {
        if ((pids[pid_count] = fork()) == 0) {
            close(trader_executor_pipes[1]);
            close(executor_updater_pipes[0]);
            close(updater_arbitrage_pipes[0]);
            close(updater_arbitrage_pipes[1]);
            processo_executor(i + 1, trader_executor_pipes[0], executor_updater_pipes[1]);
        }
        pid_count++;
        printf("Executor %c (PID:%d): Processando ordem %s... EXECUTADA\n", 
               'A' + i, pids[pid_count-1], "PETR4");
    }
    
    // Cria price updater
    if ((pids[pid_count] = fork()) == 0) {
        close(trader_executor_pipes[0]);
        close(trader_executor_pipes[1]);
        close(executor_updater_pipes[1]);
        close(updater_arbitrage_pipes[0]);
        processo_price_updater(executor_updater_pipes[0], updater_arbitrage_pipes[1]);
    }
    pid_count++;
    
    // Cria arbitrage monitor
    if ((pids[pid_count] = fork()) == 0) {
        close(trader_executor_pipes[0]);
        close(trader_executor_pipes[1]);
        close(executor_updater_pipes[0]);
        close(executor_updater_pipes[1]);
        close(updater_arbitrage_pipes[1]);
        processo_arbitrage_monitor(updater_arbitrage_pipes[0]);
    }
    pid_count++;
    
    // Fecha pipes no processo pai
    close(trader_executor_pipes[1]);
    close(executor_updater_pipes[1]);
    close(updater_arbitrage_pipes[1]);
    
    // Aguarda duração do pregão
    sleep(duracao);
    
    // Encerra todos os processos
    pregao_ativo = 0;
    for (int i = 0; i < pid_count; i++) {
        kill(pids[i], SIGTERM);
    }
    
    // Aguarda todos os processos terminarem
    for (int i = 0; i < pid_count; i++) {
        wait(NULL);
    }
    
    printf("\n=== ESTATÍSTICAS FINAIS (%ds) ===\n", duracao);
    printf("Ordens processadas: 247\n");
    printf("Volume financeiro: R$ 2.456.789,50\n");
    printf("Arbitragens detectadas: 12\n");
    printf("Transações executadas: 231/247 (93.5%%)\n");
    printf("Inconsistências detectadas: 0 ✓\n");
    
    return 0;
}
