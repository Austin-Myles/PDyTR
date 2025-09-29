#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>  // Para strlen y otras funciones de cadenas
#include <unistd.h>  // Para read, write y otras funciones de E/S
#include <stdlib.h>  // Para exit y otras funciones estándar
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

    //char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port & buffersize\n", argv[0]);
       exit(1);
    }

    // TOMA EL NÚMERO DE PUERTO DE LOS ARGUMENTOS
    portno = atoi(argv[2]);

    // TOMA LA CANTIDAD DE DATOS A COMUNICAR
    int size = atoi(argv[3]);
	
    // CREA EL FILE DESCRIPTOR DEL SOCKET PARA LA CONEXIÓN
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET - FAMILIA DEL PROTOCOLO - IPV4 PROTOCOLS INTERNET
    // SOCK_STREAM - TIPO DE SOCKET 
	
    if (sockfd < 0) 
        error("ERROR opening socket");
	
    // TOMA LA DIRECCIÓN DEL SERVIDOR DE LOS ARGUMENTOS
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // LIMPIA LA ESTRUCTURA serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	
    // COPIA LA DIRECCIÓN IP Y EL PUERTO DEL SERVIDOR A LA ESTRUCTURA DEL SOCKET
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
	
    // DESCRIPTOR - DIRECCIÓN - TAMAÑO DIRECCIÓN
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    char *buffer = malloc(size);
    memset(buffer, 'A', size);

    //Enviamos todo en un bucle porque write no suele enviar todo...
    int total_sent = 0;
    gettimeofday(&start,NULL );
    while(total_sent < size){
        n = write(sockfd, buffer + total_sent, size - total_sent);
        if (n < 0) error("ERROR writing on socket");
        total_sent += n;
    }
    gettimeofday(&end, NULL);
    free(buffer);
    close(sockfd);

    long seconds = end.tv_sec - start.tv_sec;
    long usec = end.tv_usec - start.tv_usec;
    double elapsed = seconds * 1000000.0 + usec;  // en microsegundos

    printf("Tiempo transcurrido del cliente: %.3f microsegundos\n", elapsed);
    return 0;
}
