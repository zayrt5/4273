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

struct user{
    char uid[MAXLINE], pw[MAXLINE];

};

void * thread(void * vargp);
int open_listenfd(int port);
void echo(int connfd);
int open_listenfd(int port);
int open_clientfd(char *hostname, int port);
int open_client(char *hostname, int port);
int authenticate(char* uid, char* pw, struct user *dfuser, int usercount);



int main(int argc, char **argv) 
{
    int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
    char clientip[BUFSIZ];
    
    struct sockaddr_in clientaddr;
    pthread_t tid; 
    printf("Initialized\n");
    if (argc != 3) {
        fprintf(stderr, "usage: %s <userdir> <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[2]);
    char *userdir = argv[1];
    listenfd = open_listenfd(port);
    inet_ntop( AF_INET, &clientaddr.sin_addr, clientip, BUFSIZ);
    while (1) {
        //printf("Connections being configured.\n");
        //printf("1server client ip accepted: %s\n", clientip);
        inet_ntop( AF_INET, &clientaddr.sin_addr, clientip, BUFSIZ);
        connfdp = malloc(sizeof(int));
        
        *connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);

        char *ip = inet_ntoa(clientaddr.sin_addr);
        printf("server client ip accepted: %s\n", ip);
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
    char * confname = "dfserver/dfs.conf";
    size_t n; 
    FILE *stream; //open the connfd as a stream
    char buf[MAXLINE]; 
    struct stat bufstat;
    int infd; //filedes of file to be sent to client
    //char httpmsg[]="HTTP/1.1 200 Ok\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>"; 
    char uid[50], pw[200], command[50], option[200];
    struct user users[10];
    char *userdata[15];
    int usercount = 0;

    FILE *fp = fopen(confname, "r");
    //open the connection filedes as a file stream
    if ((stream = fdopen(connfd, "r+")) == NULL)
      error(stream, "Internal Issue", 
	     "Couldn't initiate connection stream", fname, "500");

    printf("Connection established with connfd: %i\n", connfd);

    //infd = open_client("127.0.0.1", 8888);

    //load users
    
    while(strcmp(buf, "\r\n")){
        fgets(buf, BUFSIZ, fp);
        if(p = strstr(buf, "user")){
                userdata[usercount] = p + strlen("user ");
                if(userdata[usercount]){
                    p = strstr(userdata[usercount], "password ");

                    int pn = strlen(p);
                    int an = strlen(userdata[usercount]);
                    userdata[usercount][an - pn - 1] = 0;

                    strcpy(users[usercount].uid, userdata[usercount]);

                    sscanf(p, " password %s\n", users[usercount].pw);
                    
                    //printf("user: <%s>\n", userdata[usercount]);
                    //printf("pw: <%s>\n\n", users[usercount].pw);

                    usercount++;
                }
            }
    }

    fgets(buf, BUFSIZ, stream);
    //printf("preauth buf: %s\n", buf);
    sscanf(buf, "%s %s %s %s", uid, pw, command, option);
    bzero(buf, BUFSIZ);
    int usern = authenticate(uid, pw, users, usercount);
    if(usern == -1){
        printf("failed authentication\n");
        fclose(stream);
        return;
    }
    else{
        printf("Authenticated, command received: <%s> <%s>\n", command, option);
    }

    if(!strcmp(command, "LIST")){

    }
    else if(!strcmp(command, "GET")){
        
    }
    else if(!strcmp(command, "PUT")){
        while(recv(connfd, buf, BUFSIZ, 0)){
            
        }
    }
    
    
    fclose(stream);
    printf("Connection closed with connfd: %i\n", connfd);

}



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


int open_clientfd(char *hostname, int port) 
{
    int clientfd;
    

    struct hostent *hp;
    struct sockaddr_in serveraddr;
    int clientlen = sizeof(struct sockaddr_in);
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
    accept(clientfd, (struct sockaddr*) &serveraddr, &clientlen );
    printf("returning connection fd %i\n", clientfd);
    return clientfd;
}

int open_client(char *hostname, int port){
    int serverfd;
    if( (serverfd = open_clientfd(hostname, port )) < 0){
        return -1;
    }
    else{
        printf("Server connection established at %s with port %i with fd %i\n", hostname, port, serverfd);
    }

    return serverfd;
}

int authenticate(char* uid, char* pw, struct user *dfuser, int usercount){
    for(int i = 0; i < usercount; i++){
        if(!strcmp(dfuser[i].uid, uid)){
            if(!strcmp(dfuser[i].pw, pw)){
                return i;
            }

        }
        return -1;
    }
}