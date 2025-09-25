/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>   // Para exit, atoi y otras funciones estándar
#include <string.h>   // Para bzero y otras funciones de cadenas
#include <unistd.h>   // Para read, write y otras funciones de E/S
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int n, sockfd, newsockfd, portno;
     socklen_t clilen;
     //char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     struct timeval start, end;

     if (argc < 3) {
         fprintf(stderr,"ERROR, no port provided OR size of Buffer not provided\n");
         exit(1);
     }
	 
	 // CREA EL FILE DESCRIPTOR DEL SOCKET PARA LA CONEXIÓN
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 // AF_INET - FAMILIA DEL PROTOCOLO - IPV4 PROTOCOLS INTERNET
	 // SOCK_STREAM - TIPO DE SOCKET 
	 
     if (sockfd < 0) 
        error("ERROR opening socket");
		
     // LIMPIA LA ESTRUCTURA serv_addr
     bzero((char *) &serv_addr, sizeof(serv_addr));
     
     // TAMAÑO DEL BUFFER DE RECEPCIÓN
     int size = atoi(argv[2]);
    

     // ASIGNA EL PUERTO PASADO POR ARGUMENTO
     // ASIGNA LA IP EN DONDE ESCUCHA (SU PROPIA IP)
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
	 
	// VINCULA EL FILE DESCRIPTOR CON LA DIRECCIÓN Y EL PUERTO
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
			  
     // SETEA LA CANTIDAD QUE PUEDEN ESPERAR MIENTRAS SE MANEJA UNA CONEXIÓN
     listen(sockfd, 5);
	 
	 // SE BLOQUEA A ESPERAR UNA CONEXIÓN
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
				 
     // DEVUELVE UN NUEVO DESCRIPTOR POR EL CUAL SE VAN A REALIZAR LAS COMUNICACIONES
	 if (newsockfd < 0) 
          error("ERROR on accept");

    char *buffer = malloc(size);

    int total_read = 0;
    gettimeofday(&start,NULL);
    while(total_read < size){
        n = read(newsockfd, buffer + total_read, size - total_read);
        if (n < 0) error("ERROR reading from socket");
        if (n == 0) break;
        printf("Cantidad leída en esta iteración: %d bytes\n", n);
        total_read += n;
    }
    gettimeofday(&end, NULL);

    free(buffer);

     // CIERRA LOS SOCKETS
     close(newsockfd);
     close(sockfd);
     
    long seconds = end.tv_sec - start.tv_sec;
    long usec = end.tv_usec - start.tv_usec;
    double elapsed = seconds * 1000000.0 + usec;  // en microsegundos.

    printf("Tiempo transcurrido del servidor: %.3f microsegundos\n", elapsed);
     return 0; 
}
