/* 
 * tcpechosrv.c - A concurrent TCP echo server using threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <fcntl.h>
#include <sys/socket.h>  /* for socket use */
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/mman.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

#define APP_FILES "./www"

int open_listenfd(int port);
void echo(int connfd);
void *thread(void *vargp);
void error(FILE *stream, char *cause, char *errno, 
	    char *shortmsg, char *longmsg);






int main(int argc, char **argv) 
{
    int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
    

    struct sockaddr_in clientaddr;
    pthread_t tid; 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    port = atoi(argv[1]);

    listenfd = open_listenfd(port);
    while (1) {
	connfdp = malloc(sizeof(int));
	*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
	pthread_create(&tid, NULL, thread, connfdp);
    }
}



void error(FILE *stream, char *errmessage, char *errdesc, 
     char *issue, char *errno) {
  fprintf(stream, "HTTP/1.1 %s %s\n", errno, errmessage);
  fprintf(stream, "Content-type: text/html\n");
  fprintf(stream, "\n");
  fprintf(stream, "%s: %s\n", errno, errmessage);
  fprintf(stream, "<p>%s: %s\n", errdesc, issue);
}




/* thread routine */
void * thread(void * vargp) 
{  
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self()); 
    free(vargp);
    echo(connfd);
    close(connfd);
    return NULL;
}



/*
 * echo - read and echo text lines until client closes connection
 */
void echo(int connfd) 
{
    char rmethod[1024];  /* request method */
    char ruri[MAXBUF];     /* request uri */
    char rversion[1024]; /* request method */
    char fname[MAXBUF];/* path derived from uri */
    char ftype[1024]; //type of file requested
    char *p; //temporary pointer
    size_t n; 
    FILE *stream; //open the connfd as a stream
    char buf[MAXLINE]; 
    struct stat bufstat;
    int infd; //filedes of file to be sent to client
    //char httpmsg[]="HTTP/1.1 200 Ok\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>"; 

    
    //open the connection filedes as a file stream
    if ((stream = fdopen(connfd, "r+")) == NULL)
      error(stream, "Internal Issue", 
	     "Couldn't initiate connection stream", fname, "500");

    //receive the request
    fgets(buf, BUFSIZ, stream);
    //printf("server received the following request:\n%s\n",buf);
    //parse for the method uri and version
    printf("%s", buf);
    sscanf(buf, "%s %s %s\n", rmethod, ruri, rversion);


    //ignore the request headers
    while(strcmp(buf, "\r\n")) {
      fgets(buf, BUFSIZ, stream);
      printf("%s", buf);
    }
    //put www into the file amd then append the uri unless request is ./
    //then return index.html
    strcpy(fname, "www");
    strcat(fname, ruri);
    if (ruri[strlen(ruri)-1] == '/') 
	    strcat(fname, "index.html");

    //printf("filename requested: %s\n", fname);

    //make sure file exists
    if (stat(fname, &bufstat) < 0) {
      error(stream, "Not found", 
	     "Couldn't find this file" , fname, "404");
      fclose(stream);
      return;
    }





    //get file request type
    if (strstr(fname, ".html")){
	    strcpy(ftype, "text/html");
    }
    else if (strstr(fname, ".gif")){
	    strcpy(ftype, "image/gif");
    }
    else if (strstr(fname, ".jpg")){
	    strcpy(ftype, "image/jpg");
    }
    else if (strstr(fname, ".png")){
	    strcpy(ftype, "image/png");
    }
    else if (strstr(fname, ".css")){
	    strcpy(ftype, "text/css");
    }
    else if (strstr(fname, ".js")){
	    strcpy(ftype, "application/javascript");
    }
    else{ 
	    strcpy(ftype, "text/plain");
    }
    //append all headers to the stram and flush
    fprintf(stream, "HTTP/1.1 200 Ok\n");
    fprintf(stream, "Server: 4273 Get server\n");
    fprintf(stream, "Content-length: %d\n", (int)bufstat.st_size);
    fprintf(stream, "Content-type: %s\n", ftype);
    fprintf(stream, "\r\n"); 
    /*printf( "HTTP/1.1 200 Ok\n");
    printf( "Server: 4273 Get server\n");
    printf( "Content-length: %d\n", (int)bufstat.st_size);
    printf( "Content-type: %s\n", ftype);
    printf( "\r\n"); */
    fflush(stream);

    //printf("Sending HTTP message to connfd %i\n", connfd);
    //send the file using mem mapping
    infd = open(fname, O_RDONLY);
    p = mmap(0, bufstat.st_size, PROT_READ, MAP_PRIVATE, infd, 0);
    fwrite(p, 1, bufstat.st_size, stream);
    munmap(p, bufstat.st_size);

    fclose(stream); 
    //char httpmsg[]="HTTP/1.1 200 Ok\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>"; 
    //strcpy(buf,httpmsg);
    //printf("server returning a http message with the following content.\n%s\n",buf);
    //write(connfd, buf,strlen(httpmsg));
    
}

/* 
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure 
 */
int open_listenfd(int port) 
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */

