/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h> 
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h>

#define BUFSIZE 1024
/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}



int sendFile(FILE* fp, char* buf, int s)
{
    int i, len;
    if (fp == NULL) {
        strcpy(buf, "File Not Found!");
        len = strlen("File Not Found!");
        buf[len] = EOF;
        return 1;
    }
  
    char ch;
    for (i = 0; i < s; i++) {
        ch = fgetc(fp);
        buf[i] = ch;
        if (ch == EOF)
            return 1;
    }
    return 0;
}





int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  FILE* fp;
  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

    printf("parent socket created...");
  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
      
    printf("\n waiting for command... \n");
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");
    
      
    printf(" \n command received... \n");
    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("server received %lu/%d bytes: %s\n", strlen(buf), n, buf);
    
    /* 
     * sendto: echo the input back to the client 
     */
    
      
    
      
    n = sendto(sockfd, buf, strlen(buf), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
      
    
    int optint = atoi(buf);
    printf("\n switching into option: %i ... \n", optint);  
    switch(optint){
        
        case 1: //message
            printf("message inbound! booyah \n");
            bzero(buf, BUFSIZE);
            n = recvfrom(sockfd, buf, BUFSIZE, 0,
                 (struct sockaddr *) &clientaddr, &clientlen);
            if (n < 0)
              error("ERROR in recvfrom");
            
            printf("\n Message from dear client: %s \n", buf);
            
            n = sendto(sockfd, buf, strlen(buf), 0, 
               (struct sockaddr *) &clientaddr, clientlen);
            if (n < 0) 
              error("ERROR in sendto");
            
            break;
            
            
        case 2: //GET
            printf("GET command received... \n");
            bzero(buf, BUFSIZE);
            n = recvfrom(sockfd, buf, BUFSIZE, 0,
                 (struct sockaddr *) &clientaddr, &clientlen);
            if (n < 0)
              error("ERROR in recvfrom");
            
            printf("\n looking for file %s \n", buf);
            
            fp = fopen(buf, "r");
            
            if (fp == NULL)
                printf("\nFile open failed!\n");
            else
                printf("\nFile Successfully opened!\n");
            
            
            
            while(1){
            
            if(sendFile(fp,buf, BUFSIZE)){
                printf("\n File loaded, sending... \n");
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, clientlen);
                
                break;
            
                }
                
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
                bzero(buf,BUFSIZE);
                
            
            }
            
            if (fp != NULL)
                fclose(fp);
            
            
            
            
            //fopen buf; if sendfile, send to client
            
            break;
            
            
        case 3: //PUT
            printf("PUT command received...");
            break;
            
            
        case 4: //DEL
            printf("DEL command received...");
            break;
            
            
        case 5: //ls
            printf("ls command received...");
            break;
            
            
        case 6: //EXIT
            printf("EXIT command received... shutting down \n \n");
            return 0;
            
            
        default:
            printf("invalid command received... somehow..");
            break;
    
    
    }  
      
  }
}
