#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/stat.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <netdb.h>

#define p 0.4

int dropMessage(float pp) { 
    srand(time(NULL));    
    float m_num = (float)rand()/(float)(RAND_MAX);
    if (m_num < pp) {
        return 1;
    }
    return 0;
}

int main() {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket creation : ");
        exit(1);
    }

    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = if_nametoindex("wlan0");
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind : ");
        exit(1);
    }

    // wait to receive on sockfd
    while (1) {
        char buf[ETH_FRAME_LEN]; memset(buf, 0, ETH_FRAME_LEN);
        int buflen = 0;
        buflen  = recvfrom(sockfd, buf, ETH_FRAME_LEN, 0, NULL, NULL);
        if (buflen < 0) {
            perror("recvfrom : ");
            continue;
        }
        struct ethhdr *eth = (struct ethhdr *)buf;
        struct iphdr *ip = (struct iphdr *)(buf + sizeof(struct ethhdr));
        // get source MAC from from ethernet header
        char src_mac[18];
        sprintf(src_mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                eth->h_source[0], eth->h_source[1], eth->h_source[2],
                eth->h_source[3], eth->h_source[4], eth->h_source[5]);
        // get IP protocol
        int protocol = ip->protocol;
        if (protocol != 254 ) continue;
        if (dropMessage(p)) {
            // printf("Dropped message\n");
            continue;
        }
        // get source IP from IP header
        char src_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ip->saddr), src_ip, INET_ADDRSTRLEN);
        // get payload
        char* payload = buf + sizeof(struct ethhdr) + sizeof(struct iphdr);
        // printf("size of payload : %d\n",buflen);
        // fflush(stdout);

        int id;
        memcpy(&id, payload, sizeof(int));
        id = ntohl(id);

        // printf("ID: %d\n", id);
        // fflush(stdout);

        // Extracting message type from the payload (1 byte after ID)
        char msgtype;
        memcpy(&msgtype, payload + sizeof(int), sizeof(char));

        // printf("Message Type: %c\n", msgtype);
        // fflush(stdout);

        // Checking if message type is '1' (ASCII value of '1')
        if (msgtype != '0') {
            // printf("Error: Message type is not a query.\n");
            continue;
        }

        // Extracting number of domains from the payload (1 byte after message type)
        char num_domains_char;
        memcpy(&num_domains_char, payload + sizeof(int) + sizeof(char), sizeof(char));

        int num_domains = num_domains_char - '0'; // Converting char to int

        // printf("Number of Domains: %d\n", num_domains);
        // fflush(stdout);

        if (num_domains <= 0 || num_domains > 8) {
            // printf("Error: Invalid number of domains.\n");
            continue;
        }

        // Array to store domain names and their lengths
        char domains[8][32]; memset(domains, 0, 8 * 32);
        int domain_len[8] = {0};
        int offset = sizeof(int) + 2 * sizeof(char); // Offset to start of domain data

        // Extracting domain names and lengths from the payload
        for (int i = 0; i < num_domains; i++) {
            int len;
            memcpy(&len, payload + offset, sizeof(int));
            domain_len[i] = ntohl(len);
            // printf("Length: %d\n", domain_len[i]);
            // fflush(stdout);
            offset += sizeof(int);
            memcpy(domains[i], payload + offset, domain_len[i]);
            domains[i][domain_len[i]] = '\0'; // Null-terminate the domain string
            offset += domain_len[i];
            // printf("Domain %d: %s (%d)\n", i + 1, domains[i], domain_len[i]);
            // fflush(stdout);
        }

        char packet[300]; memset(packet, 0, 300);
        id = htonl(id);
        memcpy(packet, &id, sizeof(int));
        msgtype = '1';            
        offset = sizeof(int);
        memcpy(packet + offset, &msgtype, sizeof(char));
        offset += sizeof(char);
        num_domains_char = (char) ('0'+num_domains);
        memcpy(packet + offset, &num_domains_char, sizeof(char));
        offset += sizeof(char);
        for (int i = 0; i < num_domains; i++) {
            int valid_name_flag = 1;
            struct hostent *he = gethostbyname(domains[i]);
            if (he == NULL) {
                valid_name_flag = 0;
                // printf("Invalid domain name: %s\n", domains[i]);
                // continue;
            }

            char valid = (char) ('0' + valid_name_flag);
            memcpy(packet + offset, &valid, sizeof(char));
            offset += sizeof(char);
            unsigned int ip;
            if (valid_name_flag) {
                ip = *(unsigned int *)(he->h_addr_list[0]);
            } else {
                ip = 0;
            }
            ip = htonl(ip);
            memcpy(packet + offset, &ip, sizeof(unsigned int));
            offset += sizeof(unsigned int);            
        }
        // for (int i = 0; i<offset; i++) printf(" %d ", packet[i]);
        // printf("\n");
        // fflush(stdout);
        
        char ethpkt[ETH_FRAME_LEN]; memset(ethpkt, 0, ETH_FRAME_LEN);
        struct ethhdr *eth1 = (struct ethhdr *)ethpkt;
        struct iphdr *ip1 = (struct iphdr *)(ethpkt + sizeof(struct ethhdr));
        // set destination IP
        ip1->daddr = ip->saddr;
        // fill the ethernet header
        eth1->h_proto = htons(ETH_P_IP);
        // set the MAC address to source MAC
        for (int i = 0; i < 6; i++) {
            eth1->h_dest[i] = eth->h_source[i];
            // eth->h_source[i] = 0x00;
        }        
        // fill the IP header
        ip1->protocol = 254;
        // add the headers to the packet
        memcpy(ethpkt, eth1, sizeof(struct ethhdr));
        memcpy(ethpkt + sizeof(struct ethhdr), ip1, sizeof(struct iphdr));
        // add the payload
        memcpy(ethpkt + sizeof(struct ethhdr) + sizeof(struct iphdr), packet, 300);
        // send the packet
        // printf("packet : %s\n", ethpkt + sizeof(struct ethhdr) + sizeof(struct iphdr));
        sendto(sockfd, ethpkt, ETH_FRAME_LEN, 0, (struct sockaddr *)&addr, sizeof(addr));
    }
}