#!/bin/bash
# Uso: ./run_server.sh <puerto> <tam_buffer> <iteraciones> <outfile>

PORT=$1
SIZE=$2
ITERACIONES=$3
OUTFILE=$4

gcc times.c -o $HOME/times
chmod +x $HOME/times

> $OUTFILE
for ((i=1; i<=ITERACIONES; i++)); do
    echo "Iteración $i" | tee -a $OUTFILE
    $HOME/times $PORT $SIZE | tee -a $OUTFILE
    sleep 1
done
