#!/bin/bash
# Uso: ./run_client.sh <ip_servidor> <puerto> <tam_buffer> <iteraciones> <outfile>

HOST=$1
PORT=$2
SIZE=$3
ITERACIONES=$4
OUTFILE=$5

gcc timec.c -o $HOME/timec
chmod +x $HOME/timec

# Esperar a que el puerto del servidor esté disponible
echo "Esperando a que $HOST:$PORT esté disponible..."
until nc -z $HOST $PORT; do
  sleep 3
done

> $OUTFILE
for ((i=1; i<=ITERACIONES; i++)); do
    echo "Iteración $i" | tee -a $OUTFILE
    $HOME/timec $HOST $PORT $SIZE | tee -a $OUTFILE
    sleep 5
done
