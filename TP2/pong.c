#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 3) {
        fprintf(stderr,"usage %s port size\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[1]);
    int size = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("ERROR on accept");

    char *buffer = malloc(size);
    if (!buffer) error("malloc");

    int total_read = 0;
    while (total_read < size) {
        n = read(newsockfd, buffer + total_read, size - total_read);
        if (n < 0) error("ERROR reading from socket");
        if (n == 0) break;
        total_read += n;
    }

    // Reenviar (pong)
    int total_sent = 0;
    while (total_sent < total_read) {
        n = write(newsockfd, buffer + total_sent, total_read - total_sent);
        if (n < 0) error("ERROR writing to socket");
        total_sent += n;
    }

    free(buffer);
    close(newsockfd);
    close(sockfd);

    return 0; 
}
