#!/bin/bash

# --- Configurações da Simulação Final ---
TRADERS=4
EXECUTORS=2
DURACAO=60
LOG_DIR="logs_execucao"

echo "====================================================="
echo "INICIANDO SUÍTE DE TESTES DO SISTEMA DE TRADING"
echo "====================================================="

# 1. Limpeza e Compilação
make clean > /dev/null 2>&1
make all
if [ $? -ne 0 ]; then
    echo "[!] FALHA NA COMPILAÇÃO. Abortando teste."
    exit 1
fi

mkdir -p "$LOG_DIR"
rm -f $LOG_DIR/*.log

echo ""
echo "====================================================="
echo "EXECUTANDO VERSÃO COM PROCESSOS (FUNCIONAL)"
echo "====================================================="

PROCESSOS_LOG="$LOG_DIR/processos_final_run.log"
./bin/processos_exec $TRADERS $EXECUTORS $DURACAO | tee "$PROCESSOS_LOG"

echo ""
echo ""
echo "====================================================="
echo "EXECUTANDO VERSÃO COM THREADS (RACE CONDITIONS)"
echo "====================================================="

THREADS_LOG="$LOG_DIR/threads_race_condition_run.log"
./bin/threads_exec $TRADERS $EXECUTORS $DURACAO | tee "$THREADS_LOG"

echo ""
echo "====================================================="
echo "SUÍTE DE TESTES CONCLUÍDA."
echo "Logs completos salvos em: $LOG_DIR/"
echo "====================================================="
