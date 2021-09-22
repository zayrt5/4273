/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    while(1){
        printf("=====MENU====== \n Send Message:1 \n get:2 \n put:3 \n delete:4 \n exit:5 \n \n \n");
        printf("Please enter #: ");
        fgets(buf, BUFSIZE, stdin);
        int optint = atoi(buf);
        
        printf("optin choice: %i \n \n", optint);
        
        switch(optint){
            
            case 1:
                bzero(buf, BUFSIZE);
                printf("\n \n Please enter msg: ");
                fgets(buf, BUFSIZE, stdin);

                /* send the message to the server */
                serverlen = sizeof(serveraddr);
                n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen); //fix ns, wrong type
                if (n < 0) 
                  error("ERROR in sendto");

                /* print the server's reply */
                n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serveraddr, &serverlen);
                if (n < 0) 
                  error("ERROR in recvfrom");
                printf("Echo from server: %s", buf);
                
                break;
                
            case 5:
                
                return 0;
                
        }
    }
    return 0;
}
