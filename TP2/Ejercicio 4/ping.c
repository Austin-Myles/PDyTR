#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct timeval start, end;

    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port size\n", argv[0]);
       exit(1);
    }

    portno = atoi(argv[2]);
    int size = atoi(argv[3]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // Preparo mensaje
    char *buffer = malloc(size);
    if (!buffer) error("malloc");
    memset(buffer, 'A', size);

    // ---- Ping-Pong ----
    gettimeofday(&start, NULL);

    // Enviar
    int total_sent = 0;
    while (total_sent < size) {
        n = write(sockfd, buffer + total_sent, size - total_sent);
        if (n < 0) error("ERROR writing to socket");
        total_sent += n;
    }

    // Recibir la respuesta
    int total_read = 0;
    while (total_read < size) {
        n = read(sockfd, buffer + total_read, size - total_read);
        if (n < 0) error("ERROR reading from socket");
        if (n == 0) break;
        printf("Cantidad leída en esta iteración: %d bytes\n", n);
        total_read += n;
    }

    gettimeofday(&end, NULL);

    free(buffer);
    close(sockfd);

    long seconds = end.tv_sec - start.tv_sec;
    long usec = end.tv_usec - start.tv_usec;
    double elapsed = seconds * 1000000.0 + usec ;

    printf("RTT para %d bytes: %.3f ms\n", size, elapsed);

    return 0;
}
