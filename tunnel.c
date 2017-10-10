#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define MAXLINE         4096    /* max text line length */
#define LISTENQ         1024    /* 2nd argument to listen() */
#define DAYTIME_PORT    3333
#define ADDR_LEN        256
#define BUFFER_SIZE     1024

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
    int listenfd, connfd, n;
    int connectToServer = 0;
    socklen_t clientlen;
    struct sockaddr_in tunneladdr, clientaddr;
    char server_info[2][ADDR_LEN + 1];
    char recvline[MAXLINE + 1];
    char* server_ip;
    char* server_name;
    


    // create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        printf("socket error\n");
        exit(1);
    }


    // specify server address and port number
    // then bind and listenmak
    bzero(&tunneladdr, sizeof(tunneladdr));
    tunneladdr.sin_family = AF_INET;
    tunneladdr.sin_addr.s_addr = htonl(INADDR_ANY);   // IP address of server
    tunneladdr.sin_port = htons(portnum);     // server port number
    
    
    if ( bind(listenfd, (struct sockaddr *) &tunneladdr, sizeof(tunneladdr)) < 0 )
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
        
        
        // otherwise, receive ip address and port from client
        bzero(server_info, sizeof(server_info));
        int i;
        for (i = 0; i < 2; i++)
        {
            if ( (n = read(connfd, server_info[i], sizeof(server_info[i]))) > 0) 
            {
                server_info[i][n] = 0;        /* null terminate */
/*                printf("Tunnel received from client: %s\n", server_info[i]);*/
            }
                
            else if ( (n = read(connfd, server_info[i], sizeof(server_info[i]))) < 0) 
            {
                printf("read error (client->tunnel)\n");
                exit(1);
            }   
            
            if (i == 1)
            {
                connectToServer = 1;
            }
     
        }
        
        
        
        
        if (connectToServer)  // connect to server using received message
        {
            int     sockfd;
            char    serverIPAddr[BUFFER_SIZE];
            char    ipaddr[100];
            struct sockaddr_in servaddr;
            
            
            
            // create socket
            if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
            {
                printf("socket error\n");
                exit(1);
            }

            // Specify forwarding address and port number.
            // Reverse lookup forwarding ip address
            // and convert IPv4 dotted decimal address to binary, then 
            // store in forwarding.sin_addr pointer
            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(atoi(server_info[1]));  // convert port number to big endian order
            

            char* testval = server_info[0];
            if ( isdigit(testval[0]) )    // if first character is a number
            {
                // translate ip address from decimal to binary 
                if (inet_pton(AF_INET, server_info[0], &servaddr.sin_addr) <= 0)
                {
                    printf("inet_pton error for %s\n", server_info[0]);
                    exit(1);
                }

                // converts ip to machine name
                if ( getnameinfo((struct sockaddr *) &servaddr, sizeof(servaddr),
                    serverIPAddr, sizeof(serverIPAddr), NULL, 0, 0) != 0 )
                {
                    printf("reverse lookup failed\n");
                    exit(1);
                }
               
                
                
                // assign name and ip address of server to pointers
                server_name = serverIPAddr;
                server_ip = server_info[0];
                
                // establish new connection
                if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
                {
                    printf("connect error!\n");
                    exit(1);
                }
            }



            else if ( isalpha(testval[0]) ) // if first character is an alphabet
            {
                // convert machine name to ip address
                struct addrinfo hints;
                struct addrinfo *result, *node;
                
                
                bzero(&hints, sizeof(hints));
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = 0;
                if ( getaddrinfo(server_info[0], server_info[1], &hints, &result) != 0 )
                {
                    printf("reverse lookup failed\n");
                    exit(1);
                }
                
                
                // search link list for matching node
                for (node = result; node != NULL; node = node->ai_next)
                {
                    if ( (sockfd = socket(node->ai_family, node->ai_socktype, 
                        node->ai_protocol) ) < 0)
                    {
                        continue;
                    }
                    
                    
                    
                    if (connect(sockfd, node->ai_addr, node->ai_addrlen) == 0)
                    {
                        break;  // connection is established
                    }
                
                    close(sockfd);  // close socket and search again
                }
                    
                    
                // convert ip address from binary to decimal, 
                inet_ntop(AF_INET, &(node->ai_addr), ipaddr, INET_ADDRSTRLEN );
                
                // assign name and ip address of server to pointers
                server_name = server_info[0];
                server_ip = ipaddr;
                
                freeaddrinfo(result);
            }

        

            

            
            
            // receive results from server
            while ( (n = read(sockfd, recvline, MAXLINE)) > 0) 
            {
                recvline[n] = 0;        /* null terminate */
/*                if (fputs(recvline, stdout) == EOF) */
/*                {*/
/*                        printf("fputs error\n");*/
/*                        exit(1);*/
/*                }*/
            }
            
            if (n < 0) 
            {
                printf("read error\n");
                exit(1);
            }
            

                
            close(sockfd);
            connectToServer = 0;
        }
        
        
        
        // send three messages back to client
        if ( (n = write(connfd, server_name, strlen(recvline)) ) < 0 )
        {
            printf("write error\n");
            exit(1);
        }
        
        sleep(1);
        
        if ( (n = write(connfd, server_ip, strlen(recvline)) ) < 0 )
        {
            printf("write error\n");
            exit(1);
        }
        
        sleep(1);
        
        if ( (n = write(connfd, recvline, strlen(recvline)) ) < 0 )
        {
            printf("write error\n");
            exit(1);
        }
        
        sleep(1);
         

        close(connfd);
    }
}

