/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv) 
{
    int listenfd, connfd, port, error;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char haddrp[INET_ADDRSTRLEN];
    char host_name[NI_MAXHOST];
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);
    while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    /* determine the domain name and IP address of the client */
	error = getnameinfo((struct sockaddr *)&clientaddr, sizeof(clientaddr),
 	    host_name, sizeof(host_name), NULL,0, 0);
	if (error != 0) {
		fprintf(stderr, "ERROR: %s\n", gai_strerror(error));
		Close(connfd);
		continue;
	}
    inet_ntop(AF_INET, &clientaddr.sin_addr, haddrp, INET_ADDRSTRLEN);
    printf("server connected to %s (%s)\n", host_name, haddrp);

    echo(connfd);
    Close(connfd);
    }
    exit(0);
}
/* $end echoserverimain */
