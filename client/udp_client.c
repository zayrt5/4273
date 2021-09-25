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
    char buf2[BUFSIZE];
    FILE *fp;

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
        printf("\n =====MENU====== \n Send Message:1 \n get:2 \n put:3 \n delete:4 \n ls:5 \n exit:6 \n \n \n");
        
         bzero(buf, BUFSIZE);
        printf("Please enter #: ");
       
        fgets(buf, BUFSIZE, stdin);
        
        
        serverlen = sizeof(serveraddr);
        n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen); 
                
        if (n < 0) 
            error("ERROR in sendto");

        /* print the server's reply */
        n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serveraddr, &serverlen);
        
        if (n < 0) 
            error("ERROR in recvfrom");
                
        int optint = atoi(buf);
        
        //printf("optin choice: %i \n \n", optint);
        
        switch(optint){
            
            case 1:
                bzero(buf, BUFSIZE);
                printf("\n \n Please enter msg: ");
                fgets(buf, BUFSIZE, stdin);

                /* send the message to the server */
                n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen); 
                
                if (n < 0) 
                  error("ERROR in sendto");

                /* print the server's reply */
                n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serveraddr, &serverlen);
                if (n < 0) 
                  error("ERROR in recvfrom");
                printf("Echo from server: %s", buf);
                
                bzero(buf, BUFSIZE);
                
                break;
                
            case 2: //GET
                
                printf("\n GET command sent... enter file name requested: ");
                bzero(buf, BUFSIZE);
                fgets(buf, BUFSIZE, stdin);
                
                buf[strlen (buf) - 1] = '\0';
                strcpy(buf2,buf);
                n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen); 
                if (n < 0) 
                  error("ERROR in sendto");
                
                n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&serveraddr,
                                      &serverlen);
                
                
                if(strcmp(buf,"544") == 0){
                    printf("\n File not found. Aborting. \n");
                    bzero(buf, BUFSIZE);
                    break;
                }
                
                
                fp = fopen(buf2, "w");
                
                while (1) {                    
         
                    n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&serveraddr,
                                      &serverlen);
                    
                    if (strcmp(buf, "END") == 0){
                      break;
                    }
                    
                    fprintf(fp, "%s", buf);
                    bzero(buf, BUFSIZE);
                    
                }
                
                bzero(buf, BUFSIZE);
                fclose(fp);
                printf("\nFile received successfully!\n");
                break;
                
                
            case 3: //PUT
                printf("\n PUT command sent... Enter file to be sent to server: ");
                bzero(buf, BUFSIZE);
                fgets(buf, BUFSIZE, stdin);
                
                buf[strlen (buf) - 1] = '\0';
                
                
                
                
                fp = fopen(buf,"r");
                
                if (fp == NULL){
                    printf("\nFile open failed!\n");
                    strcpy(buf,"544");
                    sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);
                    bzero(buf, BUFSIZE);
                    break;
                }
                
                
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);
                
                
                while(fgets(buf, BUFSIZE, fp) != NULL){
                
                    //printf("[SENDING] Data: %s", buf);
                    
                    sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);



                    bzero(buf, BUFSIZE);



                }
                strcpy(buf,"END");
            sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, serverlen);
            fclose(fp);
            
            printf("\nFile sent successfully!\n");
                bzero(buf, BUFSIZE);
                break;
                
                
                
                
            case 4:  //DEL
                printf("DEL command sent... Enter file to be deleted:");
                bzero(buf, BUFSIZE);
                fgets(buf, BUFSIZE, stdin);
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, serverlen);
                
                n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&serveraddr,
                                      &serverlen);
                if(strcmp(buf,"END") == 0){
                    printf("\n File deleted successfully \n ");
                }
                else{
                    printf("\n File not found \n ");
                
                }
                bzero(buf, BUFSIZE);
                break;    
                
                
                
                
            case 5:   //ls
                printf("ls command sent... Enter directory: ");
                
                bzero(buf, BUFSIZE);
                fgets(buf, BUFSIZE, stdin);
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, serverlen);
                
                while(1){
                                
                    
                
                    if(strcmp(buf,"END") == 0){
                        printf("\n End of dir. \n ");
                        bzero(buf, BUFSIZE);
                        break;
                    }
                    
                    n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&serveraddr,
                                      &serverlen);
                    
                    printf(" - %s \n",buf);
                    
                    if(strcmp(buf,"END") == 0){
                        printf("\n End of dir. \n ");
                        bzero(buf, BUFSIZE);
                        break;
                    }
                    
                    bzero(buf, BUFSIZE);
                    
                }
                bzero(buf, BUFSIZE);
                break;
                
            case 6:  //EXIT
                printf("EXIT command sent... \n ");
                printf("Server shut down, closing client. \n \n");
                return 0;
            
                
            default:
                
                printf("\n options are 1-6. \n");
                
        }
    }
    return 0;
}

