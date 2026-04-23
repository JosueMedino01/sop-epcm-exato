#!/bin/bash

RUN=./app.out 

INPUT_DIR=instances
OUTPUT_DIR=results
CPLEX_OUTPUT_DIR=cplex

START_NODE=0
END_NODE=0
MIN_PRIZE=-1
MIN_PROB=-1
LIMIT_TIME=360

NUM_INSTANCES=15
SMALL_N=14
MEDIUM_N=28
LARGE_N=56

# mkdir -p $OUTPUT_DIR/small/cplex
# mkdir -p $OUTPUT_DIR/small/log
# mkdir -p $OUTPUT_DIR/medium/cplex
# mkdir -p $OUTPUT_DIR/medium/log
# mkdir -p $OUTPUT_DIR/large/cplex
# mkdir -p $OUTPUT_DIR/large/log

# echo "Executando instâncias pequenas..."

# for i in $(seq 1 $NUM_INSTANCES); do
#     INSTANCE=$INPUT_DIR/small/small_${SMALL_N}_$i.txt
    
#     CPLEX_OUT_FILE=$OUTPUT_DIR/small/cplex/small_${SMALL_N}_$i.txt
#     LOG_OUT_FILE=$OUTPUT_DIR/small/log/small_${SMALL_N}_$i.txt
    
#     echo "Rodando $INSTANCE"

#     $RUN $INSTANCE $START_NODE $END_NODE $MIN_PRIZE $MIN_PROB $LIMIT_TIME $LOG_OUT_FILE \
#         > $CPLEX_OUT_FILE
# done

# LIMIT_TIME=1800
# echo "Executando instâncias médias..."
# for i in $(seq 15 $NUM_INSTANCES); do
#     INSTANCE=$INPUT_DIR/medium/medium_${MEDIUM_N}_$i.txt
    
#     CPLEX_OUT_FILE=$OUTPUT_DIR/medium/cplex/medium_${MEDIUM_N}_$i.txt
#     LOG_OUT_FILE=$OUTPUT_DIR/medium/log/medium_${MEDIUM_N}_$i.txt
    
#     echo "Rodando $INSTANCE"

#     $RUN $INSTANCE $START_NODE $END_NODE $MIN_PRIZE $MIN_PROB $LIMIT_TIME $LOG_OUT_FILE \
#         > $CPLEX_OUT_FILE
# done

LIMIT_TIME=3600

echo "Executando instâncias grandes..."
for i in $(seq 2 $NUM_INSTANCES); do
    INSTANCE=$INPUT_DIR/large/large_${LARGE_N}_$i.txt
    
    CPLEX_OUT_FILE=$OUTPUT_DIR/large/cplex/large_${LARGE_N}_$i.txt
    LOG_OUT_FILE=$OUTPUT_DIR/large/log/large_${LARGE_N}_$i.txt
    
    echo "Rodando $INSTANCE"

    $RUN $INSTANCE $START_NODE $END_NODE $MIN_PRIZE $MIN_PROB $LIMIT_TIME $LOG_OUT_FILE \
        > $CPLEX_OUT_FILE
done