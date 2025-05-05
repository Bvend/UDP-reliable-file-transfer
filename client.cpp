// Client side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define SERVER_PORT	 8080 
#define SERVER_IP "127.0.0.1" // "192.168.18.20"
#define BUFFER_SIZE 1024 

// Driver code 
int main() { 
	int sockfd; 
	char buffer[BUFFER_SIZE]; 
	const char *message = "Hello from client!"; 
	struct sockaddr_in servaddr; 

	// Creating socket file descriptor
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) { 
		printf("socket creation failed"); 
		return 1; 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(SERVER_PORT); 
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);  
	
	// flags coul be MSG_CONFIRM
	sendto(sockfd, (const char *)message, strlen(message), 0,
		(const struct sockaddr *) &servaddr, sizeof(servaddr)); 
	printf("Message sent to server!\n"); 

	socklen_t len = sizeof(servaddr);
		
	// flags coul be MSG_WAITALL
	ssize_t n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 0, 
		(struct sockaddr *) &servaddr, &len);
	buffer[n] = '\0'; 
	printf("Server reply: %s\n", buffer); 

	close(sockfd); 
	return 0; 
}
