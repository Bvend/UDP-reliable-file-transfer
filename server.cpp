#include <bits/stdc++.h> 

#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/ioctl.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h> 
#include <net/if.h>

using namespace std;
   
#define PORT        8080 

vector<vector<char>> files;
map<string, int> clients_file;

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

string get_client_addr_as_string(struct sockaddr_in *cliaddr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((*cliaddr).sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs((*cliaddr).sin_port);

    string addr = string(client_ip) + string(":") + to_string(client_port);
    return addr;
}

// Driver code 
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
       
    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { 
        printf("socket creation failed"); 
        return 1;
    }

    int mss_size = get_mss_size(sockfd);
    printf("MSS size: %d\n", mss_size); 
       
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

    char buffer[mss_size];
    socklen_t len = sizeof(cliaddr);
    while(true) {
        printf("waiting for request\n");

        // Wait for a message from client
        // May use 0 instead of MSG_WAITALL, not sure
        int n = recvfrom(sockfd, buffer, mss_size, 0,
            (struct sockaddr *) &cliaddr, &len);

        printf("Received request\n");

        string cliaddr_str = get_client_addr_as_string(&cliaddr);
    
        if (n < 4) {
            printf("Message too short\n");
            continue;
        }

        // Safely extract 32-bit integer (network byte order)
        // uint32_t net_value1;
        // memcpy(&net_value1, buffer, 4);

        // Convert from network to host byte order
        // uint32_t ack = ntohl(net_value1);

        uint32_t ack;
        memcpy(&ack, buffer, 4);

        printf("ack: %d\n", ack);

        if (ack == 0) {
            if (n == 4) {
                printf("No file name\n");
                continue;
            }

            printf("first request\n");

            char file_name[n - 4];
            memcpy(file_name, buffer + 4, (size_t)(n - 4));

            printf("%s\n", file_name);

            std::ifstream file(file_name, std::ios::binary);

            if (!file) {
                printf("file open failed");
                continue;
            }

            std::vector<char> data(
                (std::istreambuf_iterator<char>(file)),
                (std::istreambuf_iterator<char>()));

            file.close();

            // Register connection
            files.push_back(data);
            clients_file[cliaddr_str] = files.size() - 1;

            uint32_t seq = 0;

            // Convert to network byte order
            uint32_t size = data.size();

            // Write into buffer
            memcpy(buffer,      &seq,  4);
            memcpy(buffer + 4,  &size, 4);

            // Send
            sendto(sockfd, buffer, (size_t)8, 0,
                (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
        } else {
            int client_file_num = clients_file[cliaddr_str];

            uint32_t seq = ack;

            size_t chunk_size = min((size_t)mss_size, 
                files[client_file_num].size() - seq + 1);

            memcpy(buffer, &seq, 4); // seq
            memcpy(buffer, files[client_file_num].data() + seq - 1, 
                chunk_size);

            sendto(sockfd, buffer, (size_t)(4 + chunk_size), 0, 
                (struct sockaddr *)&cliaddr, len);
        }
    }

    close(sockfd);   
    return 0; 
}
