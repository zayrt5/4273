/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

//sources
//https://idiotdeveloper.com/file-transfer-using-udp-socket-in-c/
//https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program/17683417




#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

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
  char buf2[BUFSIZE];
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  FILE* fp;
  DIR *d;
  struct dirent *dir;
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
      
    printf("\n===================\n waiting for command... \n");
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
    //printf("server received datagram from %s (%s)\n", 
	   //hostp->h_name, hostaddrp);
    //printf("server received %lu/%d bytes: %s\n", strlen(buf), n, buf);
    
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
            
            if (fp == NULL){
                printf("\nFile open failed!\n");
                strcpy(buf,"544");
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, clientlen);
                break;
            }
            else
                printf("\nFile Successfully opened!\n");
            
            
            while(fgets(buf, BUFSIZE, fp) != NULL){
                
                    //printf("[SENDING] Data: %s", buf);
                    
                    sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, clientlen);



                    bzero(buf, BUFSIZE);



                }
            strcpy(buf,"END");
            sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
            fclose(fp);
            
            printf("\nFile sent successfully!\n");
            
            
            
            
            //fopen buf; if sendfile, send to client
            
            break;
            
            
        case 3: //PUT
              printf("PUT command received...");
              n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&clientaddr,
                                      &clientlen);
                
                
               if(strcmp(buf,"544") == 0){
                    printf("\n File not found. Aborting. \n");
                    bzero(buf, BUFSIZE);
                    break;
                }
                
 
                fp = fopen(buf, "w");
                
                while (1) {                    
         
                    n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&clientaddr,
                                      &clientlen);
                    
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
            
            break;
            
            
        case 4: //DEL
            printf("DEL command received...");
            
            n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&clientaddr,
                                      &clientlen);
            
            buf[strlen (buf) - 1] = '\0';
            
            int ret = remove(buf);

            if(ret == 0) {
                    printf("File deleted successfully");
                    strcpy(buf,"END");
                    
            } 
            else {
                    printf("File not found.");
                    strcpy(buf,"544");
                    
            }
            sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
            bzero(buf, BUFSIZE);
            break;
            
            
        case 5: //ls
            printf("ls command received...");
            bzero(buf, BUFSIZE);
            
            n = recvfrom(sockfd, buf, BUFSIZE,
                                      0, (struct sockaddr*)&clientaddr,
                                      &clientlen);
            
             buf[strlen (buf) - 1] = '\0';
            
            d = opendir(buf);
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                      strcpy(buf, dir->d_name);
                      printf("%s\n", buf);
                      sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
                }
                bzero(buf, BUFSIZE);
                strcpy(buf, "END");
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
                closedir(d);
              }
            
            else{
            
                strcpy(buf, "END");
                sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
            
            }
            
            
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
