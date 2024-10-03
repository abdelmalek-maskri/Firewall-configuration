#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFERLENGTH 256

/* displays error messages from system calls */
void error(char *msg)
{
    perror(msg);
    exit(0);
}
void sendRequest(int socket, const char* request) 
{
    //printf("YOU HAVE SENT THIS TO THE SERVER: %s\n", request);
    write(socket, request, strlen(request));
}

void receiveResponse(int socket) {
    char buffer[BUFFERLENGTH];
    ssize_t bytesRead;

    while ((bytesRead = read(socket, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    if (bytesRead < 0)
    {
        perror("Failed to read from server\n");
        close(socket);
        exit(EXIT_FAILURE);
    }
}

int main (int argc, char **argv) 
{
    int socketfd;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int res;

    //printf("I JUST STARTED.\n");

    if (argc < 4) // Updated condition to check for at least 4 arguments
    {
        fprintf(stderr, "Missing arguments\n");
        exit(EXIT_FAILURE);
    }
    const char* operation = argv[3];
    
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    res = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (res != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socketfd == -1)
            continue;

        if (connect(socketfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; /* Success */

        close(socketfd);
    }

    if (rp == NULL) { /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result); /* No longer needed */

    char request[BUFFERLENGTH];
    size_t remainingSpace = sizeof(request) - strlen(request) - 1;

    request[0] = '\0';
    strncat(request, operation, remainingSpace);

    if (argc == 6) {
        strncat(request, " ", remainingSpace);
        strncat(request, argv[4], remainingSpace);
        strncat(request, " ", remainingSpace);
        strncat(request, argv[5], remainingSpace);
    }

    sendRequest(socketfd, request);

    // Receive and handle the server's response
    // bzero(request,BUFFERLENGTH);
    // int n = read(socketfd,request,BUFFERLENGTH -1);
    // if(n<0)
    // {
    //     error("Error Reading from the socket");
    // }
    // printf("%s",request);

    receiveResponse(socketfd);

    // Close the client socket
    close(socketfd);

    return 0;


}
