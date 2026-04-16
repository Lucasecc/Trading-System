 Simulador Concorrente de Trading em Bolsa

Este projeto é um simulador de um sistema de trading em bolsa de valores desenvolvido em C. Ele demonstra problemas clássicos de concorrência, como race conditions, acessos inconsistentes e ausência de sincronização em um ambiente crítico: o mercado financeiro.

 Objetivo

Simular o comportamento de uma exchange real onde múltiplos agentes (traders, executores, monitor de arbitragem e atualizador de preços) operam simultaneamente, possibilitando a análise de erros causados pela falta de controle de concorrência.

⚙️ Estrutura do Projeto

calma-la3/
├── codigo/
│   ├── main_processos.c
│   ├── main_threads.c
│   ├── trader.c
│   ├── executor.c
│   ├── price_updater.c
│   ├── arbitrage_monitor.c
│   ├── trading_system.h
├── bin/
│   ├── processos.exec
│   ├── threads.exec
├── Makefile
└── benchmark.sh

 Execução no Linux

1. Clone o repositório:
   git clone https://github.com/uPounDzZ/Trading-System.git
   cd calma-la3

2. Compile com o Makefile:
   make

3. Dê permissão de execução ao script benchmark:
   chmod +x benchmark.sh

4. Execute a versão com processos:
   ./bin/processos.exec

5. Execute a versão com threads:
   ./bin/threads.exec

 Dica: Caso tenha problemas de permissão, use: chmod +x ./bin/*.exec

 Funcionalidades

Versão com Processos:
- 4–6 processos traders gerando ordens aleatórias
- 2–3 processos executores processando ordens
- 1 processo atualizador de preços
- 1 processo monitor de arbitragem
- Comunicação via pipes anônimos

Versão com Threads:
- 4–6 threads traders
- 2–3 threads executoras
- 1 thread atualizadora de preços
- 1 thread monitora de arbitragem
- Compartilhamento de dados via estruturas globais (sem uso de mutexes)

 Estruturas de Dados

- OrdemTrading: Representa cada ordem (compra/venda)
- AcaoInfo: Informações de uma ação (preço, volume, etc.)
- EstatisticasBolsa: Agrega estatísticas gerais da simulação

 Objetivo Pedagógico

Este projeto é um exercício prático de sistemas concorrentes com o propósito de demonstrar:
- Corrupção de dados (preços inconsistentes)
- Ordens perdidas
- Falta de sincronização crítica
- Impacto direto em métricas financeiras

 Tarefas futuras (para o aluno):

✔ Analisar tecnicamente os seguintes pontos:
1. Tipos de race conditions observadas (ex: corrupção de preços, ordens perdidas)
2. Impacto das inconsistências em um sistema financeiro real
3. Diferença entre isolamento/processo vs compartilhamento/thread
4. Cenários ideais de uso de cada abordagem (processo x thread)

✔ Adicionar:
- Logs reais coletados
- Cálculo de perdas financeiras simuladas
- Propostas de métricas de qualidade (ex: tempo médio de execução, número de ordens perdidas, etc.)

