#!/bin/bash
# Uso: ./auto_cl.sh <ip_servidor> <puerto> <iteraciones> <outfile.csv>
# Ej: ./auto_cl.sh localhost 8080 10 output_cliente.csv

HOST=$1
PORT=$2
ITERACIONES=$3
OUTFILE=$4

gcc cliente.c -o $HOME/cliente
chmod +x $HOME/cliente

SIZES=(10 100 1000 10000 100000 1000000)

# Crear header CSV
echo "Iteración,10^1,10^2,10^3,10^4,10^5,10^6" > "$OUTFILE"

# Handshake inicial: verificar que el servidor esté disponible
echo "Esperando que $HOST:$PORT esté disponible..."
while ! nc -z "$HOST" "$PORT"; do
    echo "Servidor no disponible, reintentando en 1 segundo..."
    sleep 1
done

# Iteraciones como filas
for ((i=1; i<=ITERACIONES; i++)); do
    ROW="$i"

    sleep 3

    for SIZE in "${SIZES[@]}"; do
        OUTPUT=$($HOME/cliente $HOST $PORT $SIZE 1 | grep "Tiempo transcurrido del cliente")
        VAL=$(echo "$OUTPUT" | awk '{print $8}')
        ROW="$ROW,$VAL"
        sleep 1 # Pequeña pausa entre conexiones
        echo "Iteración $i, Tamaño $SIZE: $VAL microsegundos"
    done

    echo "$ROW" >> "$OUTFILE"
done
