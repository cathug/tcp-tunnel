#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>


#define MAXLINE         4096    /* max text line length */
#define DAYTIME_PORT    3333
#define BUFFER_SIZE     1024






int main(int argc, char **argv)
{
    int     sockfd;
    int     n;
    char    recvline[3][MAXLINE + 1];
    char    serverNameAddr[BUFFER_SIZE];
    char    tunnelNameAddr[BUFFER_SIZE];
    char    ipaddr[100];
    char*   server_ip;
    char*   server_name;
    char*   tunnel_ip;
    char*   tunnel_name;
    char*   testval;
    struct sockaddr_in servaddr, tunneladdr;
    
    

    // perform argument check
    if (argc != 3 && argc != 5)
    {
        printf("usage: client <serverName> <server port number> or\n \
            client <tunnelName or tunnelIP> <tunnel port Number> <serverName or serverIP> <server port number>");
        exit(1);
    }
    
    
    
    
    
    // if everything is ok, do the following

    
    // check if port number is in non-reserved range
    int portnum;
    portnum = atoi(argv[2]);
        
    if ( !(portnum > 1025 && portnum < 65535) )
    {
        printf("port number in argv[2] must be between 1025 and 65535\n");
    }
    
   
    if (argc == 3)  // if no tunnel arguments
    {
        // create socket
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        {
            printf("socket error\n");
            exit(1);
        }
    
        // Specify server address and port number.
        // Reverse lookup server ip address
        // and convert IPv4 dotted decimal address to binary, then 
        // store in servaddr.sin_addr pointer
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atoi(argv[2]));  // convert port number to big endian order
        

        char* testval = argv[1];
        // check if address is ipv4, i.e. if first character is a number
        // see RFC952 for more details
        if ( isdigit(testval[0]) )    
        {
            // translate ip address from decimal to binary 
            if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
            {
                printf("inet_pton error for %s\n", argv[1]);
                exit(1);
            }

            // converts ip to machine name
            if ( getnameinfo((struct sockaddr *) &servaddr, sizeof(servaddr),
                serverNameAddr, sizeof(serverNameAddr), NULL, 0, 0) != 0 )
            {
                printf("reverse lookup failed\n");
                exit(1);
                
            }
           
            
            // assign name and ip address of server to pointers
            server_name = serverNameAddr;
            server_ip = argv[1];
            
            
            // establish connection
            if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
            {
                printf("connect error\n");
                exit(1);
            }
        }


        // if first character is an alphabet
        else if ( isalpha(testval[0]) ) 
        {
            // convert machine name to ip address
            struct addrinfo hints;
            struct addrinfo *result, *node;
            
            
            bzero(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = 0;
            if ( getaddrinfo(argv[1], argv[2], &hints, &result) != 0 )
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
                    break;  // connection is successful
                }
            
                close(sockfd);  // close socket and search again
            }
                
                
            // convert ip address from binary to decimal, 
            inet_ntop(AF_INET, &(node->ai_addr), ipaddr, INET_ADDRSTRLEN );
            
            // assign name and ip address of server to pointers
            server_name = argv[1];
            server_ip = ipaddr;
            
            freeaddrinfo(result);
        }
        
        
        


        // print server name, ip and time
        printf("Server Name: %s\n", server_name);
        printf("Server IP Address: %s\n", server_ip);
        printf("Time: ");
        
        // receive time stamp
        bzero(&recvline, sizeof(recvline));
        while ( (n = read(sockfd, recvline[0], MAXLINE)) > 0) 
        {
            recvline[0][n] = 0;        /* null terminate */
            if (fputs(recvline[0], stdout) == EOF) 
            {
                    printf("fputs error\n");
                    exit(1);
            }
        }
        
        if (n < 0) 
        {
            printf("read error\n");
            exit(1);
        }
    }
    
    
    
    
    
    
    else  // if tunnel arguments exist, i.e. argvc == 5
    {
        // check if server port number is in non-reserved range
        portnum = atoi(argv[4]);
        
        
        if ( !(portnum > 1025 && portnum < 65535) )
        {
            printf("port number in argv[4] must be between 1025 and 65535\n");
        }
        
        
        // create socket
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        {
            printf("socket error\n");
            exit(1);
        }        
        
    
        // Specify tunnel address and port number.
        // Reverse lookup tunnel ip address
        // and convert IPv4 dotted decimal address to binary, then 
        // store in tunneladdr.sin_addr pointer
        bzero(&tunneladdr, sizeof(tunneladdr));
        tunneladdr.sin_family = AF_INET;
        tunneladdr.sin_port = htons(atoi(argv[2]));  // convert port number to big endian order

        

        testval = argv[1];
        // if first character is a number; see RFC952 for more details
        if ( isdigit(testval[0]) )    
        {
            // translate ip address from decimal to binary 
            if (inet_pton(AF_INET, argv[1], &tunneladdr.sin_addr) <= 0)
            {
                printf("inet_pton error for %s\n", argv[1]);
                exit(1);
            }

            // converts ip to machine name
            if ( getnameinfo((struct sockaddr *) &tunneladdr, sizeof(tunneladdr),
                tunnelNameAddr, sizeof(tunnelNameAddr), NULL, 0, 0) != 0 )
            {
                printf("reverse lookup failed\n");
                exit(1);
                
            }
           
        

            // assign name and ip address of tunnel to pointers
            tunnel_name = tunnelNameAddr;
            tunnel_ip = argv[1];
            
            // establish connection
            if (connect(sockfd, (struct sockaddr *) &tunneladdr, sizeof(tunneladdr)) < 0)
            {
                printf("connect error\n");
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
            if ( getaddrinfo(argv[1], argv[2], &hints, &result) != 0 )
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
                    break;  // connection is successful
                }
            
                close(sockfd);  // close socket and search again
            }
                
                
            // convert ip address from binary to decimal, 
            inet_ntop(AF_INET, &(node->ai_addr), ipaddr, INET_ADDRSTRLEN );
            
            // assign name and ip address of tunnel to pointers
            tunnel_name = argv[1];
            tunnel_ip = ipaddr;
            
            freeaddrinfo(result);
        }
        

    

        

        
        
        // send server name/ip and port to tunnel
        int i;
        for (i = 3; i < 5; i++)
        {
            if ( (n = write(sockfd, argv[i], strlen(argv[i]) )) < 0 )
            {
                printf("write error\n");
                exit(1);
            }
            
/*            printf("send server details to tunnel: %s\n", argv[i]);*/
            
            sleep(1);   // delay for 1 second
        }

        
        

        
        // receive results from tunnel
        bzero(&recvline, sizeof(recvline));
        
        
        
        // print server name, ip and time
        for (i = 0; i < 3; i++)
        {
            if ( (n = read(sockfd, recvline[i], sizeof(recvline[i]))) > 0) 
            {
                recvline[i][n] = 0;        /* null terminate */
            }
                
            else if ( (n = read(sockfd, recvline[i], sizeof(recvline[i]))) < 0)
            {
                printf("read error\n");
                exit(1);
            }   
        }
        
        
        // print server details
        printf("Server Name: %s\n", recvline[0]);
        printf("IP Address: %s\n", recvline[1]);
        printf("Time: %s\n", recvline[2]);

        
        // print tunnel details
        printf("Via Tunnel: %s\n", tunnel_name);
        printf("IP Address: %s\n", tunnel_ip);
        printf("Port Number: %s\n", argv[2]);
    }
       

    
    exit(0);
}

