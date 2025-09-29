#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct timeval start, end;

    if (argc < 5) {
        fprintf(stderr,"Uso: %s <ip_servidor> <puerto> <tam_buffer> <iteraciones>\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[2]);
    int size = atoi(argv[3]);
    int iteraciones = atoi(argv[4]);

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    char *buffer = malloc(size);
    memset(buffer, 'A', size);

    for (int i = 0; i < iteraciones; i++) {
        // Crear un socket nuevo en cada iteración
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
            error("ERROR opening socket");

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
             (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);
        serv_addr.sin_port = htons(portno);

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
            error("ERROR connecting");

        int total_sent = 0;
        gettimeofday(&start, NULL);
        while (total_sent < size) {
            n = write(sockfd, buffer + total_sent, size - total_sent);
            if (n < 0) error("ERROR writing on socket");
            total_sent += n;
        }
        gettimeofday(&end, NULL);

        long seconds = end.tv_sec - start.tv_sec;
        long usec = end.tv_usec - start.tv_usec;
        double elapsed = seconds * 1000000.0 + usec;

        printf("Iteración %d, Tiempo transcurrido del cliente (buffer=%d): %.3f microsegundos\n",
               i+1, size, elapsed);

        close(sockfd);
        usleep(100000); // opcional: 100ms de pausa entre iteraciones
    }

    free(buffer);
    return 0;
}
