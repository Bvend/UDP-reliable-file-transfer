#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h> 
#include <net/if.h>

using namespace std;

#define SERVER_PORT	8080 
#define SERVER_IP   "127.0.0.1" // "192.168.18.20"

int get_mtu_size(int sockfd) {
    int mtu_size = -1;
    ifreq ifr = {}; // net/if.h

    // Specify the interface name you want to query
    strncpy(ifr.ifr_name, "wlp1s0", IFNAMSIZ); // or "eth0", "wlan0", etc.

    if (sockfd >= 0 && ioctl(sockfd, SIOCGIFMTU, &ifr) == 0) {
        mtu_size = ifr.ifr_mtu; // in bytes
    }

    return mtu_size;
}

int get_mss_size(int sockfd) {
    // where these structures are defined
    //
    // ether_header -- net/ethernet.h -- doesn't affect MSS
    // iphdr -- netinet/ip.h
    // udphdr -- netinet/udp.h
    //
    int mss = get_mtu_size(sockfd)          
            - sizeof(iphdr)             // WARNING: this is for IPv4 only
            //- ...                     // the IP protocol accepts options!
            - sizeof(udphdr);

    return mss <= 0 ? -1 : mss;
}

// Driver code 
int main() {
    srand(time(0));
	int sockfd; 
	struct sockaddr_in servaddr; 

	// Creating socket file descriptor
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) { 
		printf("socket creation failed"); 
		return 1; 
	}

	int mss_size = get_mss_size(sockfd);
    printf("MSS size: %d\n", mss_size);

	/// memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(SERVER_PORT); 
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    string command;
    char buffer[mss_size];
    socklen_t len = sizeof(servaddr);
    while (true) {
        getline(cin, command);

        uint32_t ack = 0;
        string file_name = "file_to_send.txt";

        memcpy(buffer,      &ack,  4);
        memcpy(buffer + 4,  file_name.c_str(), (size_t)strlen(file_name.c_str()));

        // flags could be MSG_CONFIRM
        sendto(sockfd, buffer, (size_t)(4 + strlen(file_name.c_str())), 0,
            (const struct sockaddr *) &servaddr, sizeof(servaddr));

        printf("waiting for server reply\n");

        // Receive file size
        int n = recvfrom(sockfd, buffer, mss_size, 0,
            (struct sockaddr *)&servaddr, &len);

        uint32_t file_size;
        memcpy(&file_size, buffer + 4, 4);
        ack = 1; // indexado em 1

        printf("file size = %d\n", file_size);

        std::vector<char> file;

        while(ack < file_size) {
            memcpy(buffer,      &ack,  4);
            sendto(sockfd, buffer, (size_t)(4), 0,
                (const struct sockaddr *) &servaddr, sizeof(servaddr));

            n = recvfrom(sockfd, buffer, mss_size, 0,
                (struct sockaddr *)&servaddr, &len);

            uint32_t seq;

            memcpy(&seq, buffer, 4);

            file.insert(file.end(), buffer + 4, buffer + n);

            ack += n - 4;
        }
        
        file_name = "received_file" + to_string(rand()) + ".txt";
        // Save to file
        std::ofstream outfile(file_name.c_str(), std::ios::binary);
        outfile.write(file.data(), file.size());
        outfile.close();

        printf("File received\n");
    }

	close(sockfd); 
	return 0; 
}
