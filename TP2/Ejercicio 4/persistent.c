/* Servidor TCP persistente con medición de tiempo de recepción */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    struct timeval start, end;

    if (argc < 4) {
        fprintf(stderr, "Uso: %s <puerto> <tam_buffer> <iteraciones>\n", argv[0]);
        exit(1);
    }

    int size = atoi(argv[2]);
    int iteraciones = atoi(argv[3]);
    char *buffer = malloc(size);
    if (!buffer) {
        error("ERROR reservando memoria para buffer");
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // Permite reutilizar puerto rápido
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error("ERROR on setsockopt");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("Servidor persistente escuchando en puerto %d...\n", portno);

    // Loop infinito: acepta múltiples clientes
    for (int i = 0; i < iteraciones; i++) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            continue; // No matar servidor
        }

        int total_read = 0;
        gettimeofday(&start, NULL);

        while (total_read < size) {
            n = read(newsockfd, buffer + total_read, size - total_read);
            if (n < 0) error("ERROR reading from socket");
            if (n == 0) break;
            total_read += n;
        }

        gettimeofday(&end, NULL);

        close(newsockfd);

        long seconds = end.tv_sec - start.tv_sec;
        long usec = end.tv_usec - start.tv_usec;
        double elapsed = seconds * 1000000.0 + usec;  // microsegundos

        printf("Iteración %d, Tiempo transcurrido del servidor (buffer=%d): %.3f microsegundos\n",
               i+1,size, elapsed);
        fflush(stdout);
    }

    close(sockfd);
    free(buffer);
    return 0;
}
