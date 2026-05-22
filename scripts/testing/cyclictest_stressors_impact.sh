#!/bin/bash

# Default values
ITERATIONS=1000
INTERVAL_MODE=0
TICKRATE_HZ=100
STRESSORS=y
INTERVAL_START=6000
INTERVAL_END=12000
INTERVAL_STEP=200
OUTPUT=n
TICKS_MODE=1

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --iterations)     ITERATIONS=$2;     shift 2 ;;
        --interval-mode)  INTERVAL_MODE=$2;  shift 2 ;;
        --tickrate-hz)    TICKRATE_HZ=$2;    shift 2 ;;
        --stressors)      STRESSORS=$2;      shift 2 ;;
        --interval-start) INTERVAL_START=$2; shift 2 ;;
        --interval-end)   INTERVAL_END=$2;   shift 2 ;;
        --interval-step)  INTERVAL_STEP=$2;  shift 2 ;;
        --ticks-mode)     TICKS_MODE=$2;     shift 2 ;;
        --output)         OUTPUT=$2;         shift 2 ;;
        *) echo "Unknown argument: $1"; exit 1 ;;
    esac
done

cd /workspaces/cheriot-rtos/benchmarks/cyclictest || exit 1

for interval in $(seq $INTERVAL_START $INTERVAL_STEP $INTERVAL_END); do
    echo "Running config with interval=$interval"
    xmake config --sdk=/cheriot-tools/ --board=sonata \
        --iterations=$ITERATIONS \
        --interval-mode=$INTERVAL_MODE \
        --tickrate-hz=$TICKRATE_HZ \
        --stressors=$STRESSORS \
        --interval=$interval \
        --ticks-mode=$TICKS_MODE \
        --output=$OUTPUT
    read -rp "Press Enter to continue to next iteration (interval=$((interval)))..."
    xmake run

    
done