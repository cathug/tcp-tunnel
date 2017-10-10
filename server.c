#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXLINE         4096    /* max text line length */
#define LISTENQ         1024    /* 2nd argument to listen() */
#define DAYTIME_PORT    3333
#define ADDR_LEN        256

int main(int argc, char **argv)
{
    // perform argument check
    if ( argc != 2 )
    {
        printf("usage: server <portNumber>\n");
        exit(1);
    }
    
    
    // check if port number is in non-reserved range
    int portnum;
    portnum = atoi(argv[1]);
    if ( !(portnum > 1025 && portnum < 65535) )
    {
        printf("port number in argv[2] must be between 1025 and 65535\n");
    }
    
    
    // if everything is ok, do the following
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in servaddr, clientaddr;
    char buff[MAXLINE];
    char client_ip[ADDR_LEN];   // client ip address
    char clientname[ADDR_LEN];  // client name
    time_t ticks;


    // create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        printf("socket error\n");
        exit(1);
    }


    // specify server address and port number
    // then bind and listen
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   // IP address of server
    servaddr.sin_port = htons(portnum);             // server port number
    
    
    if ( bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
    {
        printf("Server failed to assign local protocol address to socket\n");
	    exit(1);
    };

    
    if ( listen(listenfd, LISTENQ) < 0 )
    {
        printf("Server failed to convert socket to passive mode.\n");
        exit(1);
    }


    // accept any connection requests
    for ( ; ; ) 
    {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        if ( connfd < 0 )
        {
            printf("Failed to accept connection request from client\n");
            exit(1);
        }
        
        if ( inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip, ADDR_LEN) == NULL)
        {
            printf("Failed to translate from binary to presentation IP address \n");
            exit(1);
        }
        
        if ( getnameinfo((struct sockaddr *) &clientaddr, sizeof(clientaddr),
             clientname, sizeof(clientname), NULL, 0, 0) != 0 )
        {
            printf("reverse lookup failed\n");
            exit(1);
        }

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        printf("------------------------------------------------------\n");
        printf("Requesting Client Name: %s\n", clientname);
        printf("Requesting Client IP address: %s\n", client_ip);
/*        printf("Sending response: %s\n", buff);*/

        close(connfd);
    }
}

