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
int open_serverfd(char *hostname, int port);
int open_server(char *hostname, int port, FILE* fstream);

//get server side working
//research web client requests
//at least get client->proxy requests flowin
 
//get socket, thread for ip and port
//listen for request on argv port no, else 80
//parse, check in cache for page. check expiration
//else check in hosts cache for host ip (avoid dns). use if cached
//have time going for specified argv for cache timeout
//create request for server, cache results and serve to client.

//client <-> proxy <-> server


int main(int argc, char **argv) 
{
    int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
    char clientip[BUFSIZ];

    struct sockaddr_in clientaddr;
    pthread_t tid; 

    port = atoi(argv[1]);
    listenfd = open_listenfd(port);
    printf("\nListening on port %i!\n", port);
    while (1) {
    //inet_ntop( AF_INET, &clientaddr.sin_addr, clientip, BUFSIZ);
    //printf("New client address: %s\n", clientip);
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
    char furi[MAXBUF];
    char rversion[1024]; /* request method */
    char rhost[MAXBUF];
    char rport[1024];
    char fname[MAXBUF];/* path derived from uri */
    char ftype[1024]; //type of file requested
    char *p; //temporary pointer
    char *p2; //needed because i am not optimizing this
    size_t n; 
    FILE *cstream; //open the connfd as a stream
    FILE *sstream; //server file stream
    char buf[MAXLINE]; 
    char garbuf[2*MAXBUF];
    struct stat bufstat;
    int serverfd; //in conjunction with connfd
    //char httpmsg[]="HTTP/1.1 200 Ok\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>"; 

    
    //open the client filedes as a file stream
    if ((cstream = fdopen(connfd, "r+")) == NULL)
      error(cstream, "Internal Issue", 
	     "Couldn't initiate client connection stream", fname, "500");

    //receive the request
    fgets(buf, BUFSIZ, cstream);
    printf("\n=====Start of proxy-client connection request %i ====\n", connfd);
    //printf("server received the following request:\n%s\n",buf);
    //parse for the method uri and version
    printf("%s\n", buf);
    //get that non GET stuff outta here
    if(!(strstr(buf, "GET"))){
        error(cstream, "Internal Issue", "Proxy only supports GET requests", buf, "500");
        printf("client connection booted:  %i\n", connfd);
        fclose(cstream);
        return;
    }

    
    sscanf(buf, "%s %s %s\n", rmethod, ruri, rversion);
    strcpy(furi, ruri); //too many char *s that i messed with ruri accidentlally
    printf("\nrmethod requested: %s", rmethod);
    printf("\nruri requested: %s", ruri);
    printf("\nrversion requested: %s\n", rversion);

    //int hostfound = 0;
    while(strcmp(buf, "\r\n")) {
      fgets(buf, BUFSIZ, cstream);
      printf("%s", buf);
    }


    

    
    //p==rh at this line
    //strcat(fname, ruri);

    //if else chains to check protocols, ports, and host via ':' and '/' delimiters
    if(p = index(ruri, ':')){
        //port and no protocol
        //printf("\none ':' found in uri\n");

        if(p[1] != '/'){
            //printf("\nport specified: %s\n", p+1);
            p++;
            strcpy(rport, p);
            if(p = strchr(rhost, '/')){
                strcpy(rhost, p);
            }
            //open connection

            if(!(serverfd = open_server(rhost, atoi(rport), cstream))){
                printf("\nserver connection booted:  %i\n", serverfd);
                fclose(cstream);
                return;
            }
        } else{
            //protocol and port
            
            p = &p[3];
            //printf("\nport and host specified for %s\n", p);
            if(p2 = strstr(p, ":")){
                p2++; //gives port pointer
                //printf("\nport specified %s\n", p2);
                strcpy(rport, p2);
                p = strchr(p, ':');
                //printf("\nhost specified %s\n", p);
                strcpy(rhost, p);
            }
            else{
                printf("\nport 80 set\n");
                
                strcpy(rport, "80");
                strcpy(rhost, p);
                //printf("\nhost specified (p) %s\n", p);
                //printf("\nhost specified (rhost) %s\n", rhost);
            }
            //isolate hostname
            if(p2 = strstr(p, "/")){
                p2[0] = '\0';
            }
            strcpy(rhost, p);
            //printf("HOST requested (p): %s\n", p);
            printf("\nHOST requested: %s\n", rhost);
            printf("PORT requested: %s\n", rport);
            if((serverfd = open_server(rhost, atoi(rport), cstream)) < 0){
                printf("\nserver connection booted:  %i\n", serverfd);
                fclose(cstream);
                return;
            }
        }
    }
    //no port or protocol specified
    else{
        strcpy(rport, "80");
        //if( serverfd = open_serverfd(rhost, atoi(rport) ) < 0){
            //error(stream, "Server connection failed", "The proxy could not start a connection with the server requested", rhost, "500" );
        //}
        //else{
            //printf("Server connection established on port %s\n", rhost);
        //}
        //handle for when we fuck w the real
    }
    if ((sstream = fdopen(serverfd, "r+")) == NULL)
      error(sstream, "Proxy Issue", 
	     "Couldn't initiate server connection stream", fname, "500");

    printf("serverfd %i connected with client fd %i\n", serverfd, connfd);

    printf("\nsending requested uri: %s\n", furi);
    //format buf to send to server
    bzero(garbuf, sizeof(garbuf));
    sprintf(garbuf, "GET %s HTTP/1.0\r\n\r\n", furi);
    strcpy(buf, garbuf);
    printf("Sending server request:\n%s", buf);
    //dont need file stream because requests shouldnt be too large
    send(serverfd, buf, strlen(buf), 0);

    //printf("Received from server: \n");


    //had to look up how to send responses larger than 8192
    //threw some of my own stuff in there
    void *clientdata = 0;
    size_t ndata = 0;
    ssize_t L = 0;
    //listen for resposnse
    bzero(buf, sizeof(buf));
    while (recv(serverfd, buf, BUFSIZ, 0) > 0) {
        //printf("%s", buf);

        L = sizeof(buf);
        if( L > 0){
            if (clientdata){
                clientdata = realloc(clientdata, ndata + L);
            }
            else{
                clientdata = malloc((unsigned long)L);
            }
            //printf("%s", buf);
            
            
            memcpy(clientdata + ndata, buf, L);
            ndata += (size_t)L;
        }  
        else{
            break;
        }

        memset(buf, 0, BUFSIZ);
    }
    printf("\nSending %zu bytes from server\n", ndata);
    write(connfd, clientdata, ndata); //heck yeah brother

    
    printf("\ndata processed, closing client and server streams!\n");
    fclose(cstream); 
    fclose(sstream); 
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

//sourced from same code as above
//opens server connection with specified host and port
int open_serverfd(char *hostname, int port) 
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1; /* check errno for cause of error */

    /* Fill in the server's IP address and port */
    if ((hp = gethostbyname(hostname)) == NULL)
	return -2; /* check h_errno for cause of error */
    memset((char *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memcpy((char *)&serveraddr.sin_addr.s_addr, 
		(char *)hp->h_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd,  (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0)
	    return -1;

    printf("returning connection fd %i\n", clientfd);
    return clientfd;
}

int open_server(char *hostname, int port, FILE* fstream){
    int serverfd;
    if( (serverfd = open_serverfd(hostname, port )) < 0){
        error(fstream, "Server connection failed", "The proxy could not start a connection with the server requested", hostname, "500" );
        return -1;
    }
    else{
        printf("Server connection established at %s with port %i with fd %i\n", hostname, port, serverfd);
    }

    return serverfd;
}
