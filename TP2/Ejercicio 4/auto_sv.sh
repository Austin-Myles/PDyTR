#!/bin/bash
# Uso: ./auto_sv.sh <puerto> <iteraciones> <outfile.csv>
# Ej: ./auto_sv.sh 2020 10 output_server.csv

PORT=$1
ITERACIONES=$2
OUTFILE=$3

gcc persistent.c -o $HOME/persistent
chmod +x $HOME/persistent

SIZES=(10 100 1000 10000 100000 1000000)

# Crear header CSV
echo "Iteración,10^1,10^2,10^3,10^4,10^5,10^6" > "$OUTFILE"

$HOME/persistent $PORT 1 1 | grep "Tiempo transcurrido del servidor" # Lanzar cliente en background
sleep 1 # Esperar a que el cliente inicie y se conecte

# Iteraciones como filas
for ((i=1; i<=ITERACIONES; i++)); do
    # Cada columna es un tamaño de buffer
    ROW="$i"

    for SIZE in "${SIZES[@]}"; do
        OUTPUT=$($HOME/persistent $PORT $SIZE 1 | grep "Tiempo transcurrido del servidor")
        VAL=$(echo "$OUTPUT" | awk '{print $8}')
        ROW="$ROW,$VAL"
        echo "Iteración $i, Tamaño $SIZE: $VAL microsegundos"
    done

    echo "$ROW" >> "$OUTFILE"
done
