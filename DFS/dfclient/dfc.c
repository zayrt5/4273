#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>  /* for socket use */
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/mman.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include <openssl/md5.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */


struct dfservers {
    char ip[MAXLINE], dir[MAXLINE], port[MAXLINE];
    int count;
};

struct user{
    char uid[MAXLINE], pw[MAXLINE];

};

void echo(int connfd);
int open_listenfd(int port);






int main(int argc, char **argv) 
{   
    struct dfservers confservers[4];
    struct user confuser[10];
    FILE *stream[12];
    char buf[MAXLINE];
    char* p, *p2;
    int listenfd, *connfdp, serverfdp[30], port, serverlen=sizeof(struct sockaddr_in);
    char serverip[BUFSIZ];
    char *serverdata[4];
    char *userdata[15];
    char *pieces[4];
    int confservercount = 0;
    int usercount = 0;

    MD5_CTX mdContext;
    int md5_table[4][4][2] =
	{
		{
			{0,1},{1,2},{2,3},{3,0}
		},
		{
			{3,0},{0,1},{1,2},{2,3}
		},
		{
			{2,3},{3,0},{0,1},{1,2}
		},
		{
			{1,2},{2,3},{3,0},{0,1}
		}
	};
    
    
    struct sockaddr_in serveraddr[4];
    printf("Initialized\n");
    if (argc != 2) {
        fprintf(stderr, "usage: %s dfc.conf \n", argv[0]);
        exit(0);
    }
    char* confname = argv[1];
    FILE *fp = fopen(confname, "r");
            
    if (fp == NULL){
        printf("\nconf open failed!\n");
        return 0;
    }

    printf("\nconf open successful!\n");
    //extracts user and server info from conf
    //very hard coded
    while(strcmp(buf, "\r\n")){
        fgets(buf, BUFSIZ, fp);
        if(p = strstr(buf, "server")){
            
            serverdata[confservercount] = (p + strlen("server ")); 
            if(serverdata[confservercount]){
                if(!sscanf(serverdata[confservercount], "%s %s", confservers[confservercount].dir, confservers[confservercount].ip)){
                    printf("failed to load directory\n");
                    return 0;
                }
 
                p = strstr(confservers[confservercount].ip, ":") + 1;
                strcpy(confservers[confservercount].port, p);
                printf("port: <%s>\n", confservers[confservercount].port);
                //pn is the :<port> length
                //an is the whole string length
                //set the difference to 0 to extract ip

                int pn = strlen(confservers[confservercount].port) + strlen(":");
                int an = strlen(confservers[confservercount].ip);
                confservers[confservercount].ip[an - pn] = 0;
                //printf("an - pn: %i %i\n", an, pn);
                printf("ip: <%s>\n", confservers[confservercount].ip);
                printf("dir: <%s>\n\n", confservers[confservercount].dir);
                confservercount++;
            }
        }

        else if(p = strstr(buf, "user")){
            userdata[usercount] = p + strlen("user ");
            if(userdata[usercount]){
                p = strstr(userdata[usercount], "password ");

                int pn = strlen(p);
                int an = strlen(userdata[usercount]);
                userdata[usercount][an - pn - 1] = 0;

                strcpy(confuser[usercount].uid, userdata[usercount]);

                sscanf(p, " password %s\n", confuser[usercount].pw);
                
                printf("user: <%s>\n", userdata[usercount]);
                printf("pw: <%s>\n\n", confuser[usercount].pw);

                usercount++;
            }
        }
    }

    printf("user: %i\nservers: %i\n", usercount, confservercount);


    //listening from the servers on 8888
    listenfd = open_listenfd(8888);
    /*for(int i = 0; i < confservercount; i++){
        listenfd[i] = open_listenfd(confservers[i]);
    }*/

    //allow connections to be made by server to c port 8888
    printf("serverip and port: %s , %i\n", serverip, port);
    //try n move while loop
        //printf("Connections being configured.\n");
        connfdp = malloc(sizeof(int));
        printf("while s addr %s configured\n", serverip);
        for(int i = 0; i < confservercount; i++){ 

            bzero(&serveraddr[i],sizeof(serveraddr[i]));               //zero the struct
            serveraddr[i].sin_family = AF_INET;                 //address family
            serveraddr[i].sin_port = htons(atoi(confservers[i].port));      //sets port to network byte order
            serveraddr[i].sin_addr.s_addr = inet_addr(confservers[i].ip); //sets remote IP address
            if ((serverfdp[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                printf("unable to create socket");
            }
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            if (setsockopt (serverfdp[i], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
                    //error("setsockopt failed\n");
                    printf("server socket error\n");
            if (connect(serverfdp[i], (struct sockaddr *) &serveraddr[i], sizeof(serveraddr[i]))<0) {
                perror("Problem in connecting to the server");
                exit(3);
            }

            printf("connected to ip %s\n", confservers[i].ip);

            if ((stream[i] = fdopen(serverfdp[i], "r+")) == NULL)
                //error
                perror("Problem in opening connection as stream");
            

            printf("stream open success\n");
            
            
        }

        //printf("pre connfd out for  %d configured\n", serverfdp[0]);
        //pthread_create(&tid, NULL, thread, &serverfdp[0]);
        //*connfdp = accept(serverfdp, (struct sockaddr*)&serveraddr, &serverlen);

        printf("enter command name and/or options:\n\n");

        fgets(buf, 250, stdin);
        char input[50], option[200];
        sscanf(buf, "%s %s\n\n", input, option);
        //GET
        if(p = strstr(input, "GET")){
            printf("in GET\n");

        }
        //LIST
        else if(p = strstr(input, "LIST")){
            printf("in LIST\n");
        }
        //PUT
        else if(p = strstr(input, "PUT")){
            printf("in PUT: %s\n", p);

            strcpy(buf, confuser[0].uid);
            strcat(buf, " ");
            
            strcat(buf, confuser[0].pw);
            strcat(buf, " ");

            strcat(buf, input);
            strcat(buf, " ");
            
            strcat(buf, option);
            //printf("buf: %s\n", buf);

            for (int i = 0; i < confservercount; i++)
            {
                //send server user info and command info
                sendto(serverfdp[i], buf, strlen(buf), 0, (struct sockaddr *)&serveraddr[i], serverlen);
                //recvfrom(serverfdp[i], buf, strlen(buf), 0, (struct sockaddr *)&serveraddr[i], &serverlen);
                
            }
            strcpy(input, "dfclient/");
            strcat(input, option);
            printf("req file: %s\n", input);
            fp = fopen(input,"r");

            if(fp == NULL){
                printf("file open failed\n");
                for (int i = 0; i < confservercount; i++){
                    fclose(stream[i]);
                } return 0;
            }

            printf("file opened\n");
            MD5_Init (&mdContext);
            //jesus christ
            //you guys are sadistic. but i like it. but i cant finish all of it
            fseek(fp, 01, SEEK_END);
            long fpsize = ftell(fp); 
            rewind(fp);
            int fp4 = fpsize/4;

            printf("sizes: %ld %d\n", fpsize, fp4 );
            for(int i = 0; i < confservercount; i++){
                if(fpsize%2 != 0 || i == confservercount -1){
                    printf("odd modulus\n");
                    
                    pieces[i] = calloc(sizeof(fp4)+1, sizeof(char));
                }
                else{
                    printf("even modulus\n");
                    pieces[i] = calloc(sizeof(fp4), sizeof(char));
                    
                }
            }
            int pieceno = 0;
            int chari, cno;
            while((chari = fgetc(fp)) != EOF ){
                printf("%c", (char) chari);
                pieces[pieceno][cno++] = (char) chari;
                
                MD5_Update (&mdContext, &chari, 1);
                if( pieceno <= 2){
                    if(cno >= fp4){
                        cno = 0;
                        pieceno++;
                    }
                }
	        }
            printf("outofloop\n");
            unsigned char mdout[MD5_DIGEST_LENGTH];
            MD5_Final (mdout,&mdContext);
            char *buf2;
            unsigned char* hash;
            hash = calloc(sizeof(MD5_DIGEST_LENGTH), sizeof(char));
            for (int i=0; i < MD5_DIGEST_LENGTH; i++){
                sprintf(buf2, "%02x",(unsigned int)mdout[i]);
                strcat(hash, buf2 );
            }
            printf("hash: %s\n", hash);
            
            munmap(pieces, fpsize);

        }
        else{
            printf("USAGE:\nGET <file>\nPUT <file>\nLIST\n");
        }
        
        for (int i = 0; i < confservercount; i++){
            fclose(stream[i]);
        }
        
        fclose(fp);
        
    
}



void error(FILE *stream, char *errmessage, char *errdesc, 
     char *issue, char *errno) {
    fprintf(stream, "HTTP/1.1 %s %s\n", errno, errmessage);
    fprintf(stream, "Content-type: text/html\n");
    fprintf(stream, "\n");
    fprintf(stream, "%s: %s\n", errno, errmessage);
    fprintf(stream, "<p>%s: %s\n", errdesc, issue);
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
    int serverfd;
    size_t n; 
    FILE *stream; //open the connfd as a stream
    char buf[MAXLINE]; 
    struct stat bufstat;
    int infd; //filedes of file to be sent to client
    //char httpmsg[]="HTTP/1.1 200 Ok\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>"; 

    
    //serverfd = open_server(rhost, atoi(rport), cstream)

    //open the connection filedes as a file stream
    /*if ((stream = fdopen(connfd, "r+")) == NULL)
      error(stream, "Internal Issue", 
	     "Couldn't initiate connection stream", fname, "500");

    printf("MADE TO ECHO WITH CONNFD: %i", connfd);
    fgets(buf, BUFSIZ, stream);
    //while(1);
    fclose(stream); */
    printf("Connection closed with connfd: %i", connfd);

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