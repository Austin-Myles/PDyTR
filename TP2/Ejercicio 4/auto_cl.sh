#!/bin/bash
# Uso: ./auto_cl.sh <ip_servidor> <puerto> <iteraciones> <outfile.csv>
# ./auto_cl.sh localhost 2020 10 output_cliente.csv
HOST=$1
PORT=$2
ITERACIONES=$3
OUTFILE=$4

gcc cliente.c -o $HOME/cliente
chmod +x $HOME/cliente

# Buffer sizes: 10^1 .. 10^6
SIZES=(10 100 1000 10000 100000 1000000)

# Crear header CSV
echo "Iteración,10^1,10^2,10^3,10^4,10^5,10^6" > "$OUTFILE"

# Iteraciones como filas
for ((i=1; i<=ITERACIONES; i++)); do
    ROW="$i"

    while (! nc -z "$HOST" "$PORT";) do
        echo "Esperando que $HOST:$PORT esté disponible..."
        sleep 1
    done
    echo "Conexión establecida!"

    # Cada columna es un tamaño de buffer
    for SIZE in "${SIZES[@]}"; do
        OUTPUT=$($HOME/cliente $HOST $PORT $SIZE 1 | grep "Tiempo transcurrido del cliente")
        VAL=$(echo "$OUTPUT" | awk '{print $8}') # porque el formato lleva "microsegundos"
        ROW="$ROW,$VAL"
    done

    echo "$ROW" >> "$OUTFILE"
done
