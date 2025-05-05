// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
   
#define PORT     8080 
#define BUFFER_SIZE 1024 
   
// Driver code 
int main() { 
    int sockfd; 
    char buffer[BUFFER_SIZE]; 
    const char *reply = "Message received by server!"; 
    struct sockaddr_in servaddr, cliaddr; 
       
    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { 
        printf("socket creation failed"); 
        return 1;
    } 
       
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
       
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
       
    // Bind the socket with the server address 
    if (bind(sockfd, (const struct sockaddr *)&servaddr, 
        sizeof(servaddr)) < 0) { 
        printf("bind failed"); 
        close(sockfd);
        return 1; 
    }

    printf("Server listening on port %d...\n", PORT);
       
    socklen_t len = sizeof(cliaddr);
    // May use 0 instead of MSG_WAITALL, not sure
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL,
        (struct sockaddr *) &cliaddr, &len); 

    buffer[n] = '\0'; 
    printf("Received from client: %s\n", buffer);

    sendto(sockfd, (const char *)reply, strlen(reply), MSG_CONFIRM, 
        (const struct sockaddr *) &cliaddr, len); 

    close(sockfd);   
    return 0; 
}
