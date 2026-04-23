#!/bin/bash

# ============================
# Configurações
# ============================

GENERATOR=./generate.out

PROB_MIN=0.3
PROB_MAX=1.0

SMALL_N=14
MEDIUM_N=28
LARGE_N=56

NUM_INSTANCES=15

OUTPUT_DIR=instances

# ============================
# Preparação
# ============================

mkdir -p $OUTPUT_DIR/small
mkdir -p $OUTPUT_DIR/medium
mkdir -p $OUTPUT_DIR/large

echo "Gerando instâncias..."

# ============================
# Pequenas
# ============================

for i in $(seq 1 $NUM_INSTANCES); do
    $GENERATOR $SMALL_N $PROB_MIN $PROB_MAX $i \
    $OUTPUT_DIR/small/small_${SMALL_N}_$i.txt
done

# ============================
# Médias
# ============================

for i in $(seq 1 $NUM_INSTANCES); do
    $GENERATOR $MEDIUM_N $PROB_MIN $PROB_MAX $((i+100)) \
    $OUTPUT_DIR/medium/medium_${MEDIUM_N}_$i.txt
done

# ============================
# Grandes
# ============================

for i in $(seq 1 $NUM_INSTANCES); do
    $GENERATOR $LARGE_N $PROB_MIN $PROB_MAX $((i+200)) \
    $OUTPUT_DIR/large/large_${LARGE_N}_$i.txt
done

echo "✔ Todas as instâncias foram geradas!"