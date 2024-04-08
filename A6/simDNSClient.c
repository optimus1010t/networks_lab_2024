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
#include <ctype.h>

#define TIME_TO_WAIT 5

struct left_to_receive {
    int id;
    int send_freq;
    char data[300];
    struct timeval last_sent;
    struct left_to_receive *next;
};

int isValidDomain(const char *domain) {
    int len = strlen(domain);
    // Check length
    if (len < 3 || len > 31)
        return 0;
    // Check characters
    for (int i = 0; i < len; i++) {
        if (!isalnum(domain[i]) && domain[i] != '-' && domain[i] != '.')
            return 0;
        if (domain[i] == '-' && (i == 0 || i == len - 1 || domain[i + 1] == '-'))
            return 0;
        if (domain[i] == '.' && (i == 0 || i == len - 1 || domain[i + 1] == '.'))
            return 0;
    }
    return 1;
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

    struct left_to_receive *head = malloc(sizeof(struct left_to_receive));
    head->id = -1;
    head->send_freq = -1;
    head->last_sent.tv_sec = 0;
    head->last_sent.tv_usec = 0;
    head->next = NULL;
    // ???? bind this
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(0, &readfds);
        struct timeval tv;
        tv.tv_sec = TIME_TO_WAIT;
        tv.tv_usec = 0;
        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) < 0) {
            perror("select : ");
            exit(1);
        }
        // check the left_to_receive and resend packets if needed
        struct left_to_receive *curr = head->next;
        struct left_to_receive *prev = head;
        struct timeval curr_time;
        gettimeofday(&curr_time, NULL);
        while (curr != NULL) {
            if (curr->send_freq == 3) {
                // delete the node
                prev->next = curr->next;
                free(curr);
                curr = prev->next;
                continue;
            }
            if (curr_time.tv_sec - curr->last_sent.tv_sec >= TIME_TO_WAIT) {
                
                char ethpkt[ETH_FRAME_LEN]; memset(ethpkt, 0, ETH_FRAME_LEN);
                struct ethhdr *eth = (struct ethhdr *)ethpkt;
                struct iphdr *ip = (struct iphdr *)(ethpkt + sizeof(struct ethhdr));
                // fill the ethernet header
                eth->h_proto = htons(ETH_P_IP);
                // set the MAC address to broadcast
                for (int i = 0; i < 6; i++) eth->h_dest[i] = 0xff;
                // fill the IP header
                ip->protocol = 254;
                // add the headers to the packet
                memcpy(ethpkt, eth, sizeof(struct ethhdr));
                memcpy(ethpkt + sizeof(struct ethhdr), ip, sizeof(struct iphdr));
                // add the payload
                memcpy(ethpkt + sizeof(struct ethhdr) + sizeof(struct iphdr), curr->data, 300);
                // send the packet
                // printf("Resending query ID %d\n", curr->id);
                sendto(sockfd, ethpkt, ETH_FRAME_LEN, 0, (struct sockaddr *)&addr, sizeof(addr));
                curr->last_sent = curr_time;
                curr->send_freq++;
            }
            curr = curr->next;
        }

        if (FD_ISSET(0, &readfds)) {
            char input[257]; memset(input, 0, 257);
            fgets(input, 256, stdin);
            
            char domains[8][32]; memset(domains, 0, 8 * 32);
            int domain_len[8]; for (int i = 0; i < 8; i++) domain_len[i] = 0;
            int num_domains = 0;
            char *token = strtok(input, " ");
            if (strcmp(token, "getIP") != 0) {
                printf("Error: Invalid input format.\n");
                continue;
            }
            token = strtok(NULL, " \n");
            if (sscanf(token, "%d", &num_domains) != 1 || num_domains > 8) {
                printf("Error: Invalid number of domains.\n");
                continue;
            }
            if (num_domains == 0) {
                printf("Error: No domains provided.\n");
                continue;
            }
            int flag = 0;
            for (int i = 0; i < num_domains; i++) {
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Error: Insufficient domains provided.\n");
                    flag = 1;
                    break;
                }
                if (!isValidDomain(token)) {
                    printf("Error: Invalid domain format for domain %d.\n", i + 1);
                    flag = 1;
                    break;
                }
                strcpy(domains[i], token);
                domain_len[i] = strlen(token);
            }
            if (flag == 1) continue;

            // create the packet
            char packet[300]; memset(packet, 0, 300);
            // add the ID in the start of 4 bytes
            int id = rand();
            // check if its in the pending queries list
            while (1) {
                int flag = 0;
                struct left_to_receive *curr = head->next;
                while (curr != NULL) {
                    if (curr->id == id) {
                        flag = 1;
                        break;
                    }
                    curr = curr->next;
                }
                if (flag == 0) break;
                id = rand();
            }
            id = htonl(id);
            memcpy(packet, &id, sizeof(int));
            char msgtype = '0';            
            int offset = sizeof(int);
            memcpy(packet + offset, &msgtype, sizeof(char));
            offset += sizeof(char);
            char num_domains_char = (char) ('0'+num_domains);
            memcpy(packet + offset, &num_domains_char, sizeof(char));
            offset += sizeof(char);
            for (int i = 0; i < num_domains; i++) {
                int len = htonl(domain_len[i]);
                // int len = domain_len[i];
                memcpy(packet + offset, &len, sizeof(int));
                offset += sizeof(int);
                memcpy(packet + offset, domains[i], domain_len[i]);
                offset += domain_len[i];
                // printf("domain : %s of length %d\n", domains[i], domain_len[i]);
                // fflush(stdout);
            }
            // for (int i = 0; i<offset; i++) printf(" %d ", packet[i]);
            // printf("\n");
            // fflush(stdout);
            
            char ethpkt[ETH_FRAME_LEN]; memset(ethpkt, 0, ETH_FRAME_LEN);
            struct ethhdr *eth = (struct ethhdr *)ethpkt;
            struct iphdr *ip = (struct iphdr *)(ethpkt + sizeof(struct ethhdr));
            // fill the ethernet header
            eth->h_proto = htons(ETH_P_IP);
            // set the MAC address to broadcast
            for (int i = 0; i < 6; i++) eth->h_dest[i] = 0xff;
            // fill the IP header
            ip->protocol = 254;
            // add the headers to the packet
            memcpy(ethpkt, eth, sizeof(struct ethhdr));
            memcpy(ethpkt + sizeof(struct ethhdr), ip, sizeof(struct iphdr));
            // add the payload
            memcpy(ethpkt + sizeof(struct ethhdr) + sizeof(struct iphdr), packet, 300);
            // send the packet
            // printf("packet : %s\n", ethpkt + sizeof(struct ethhdr) + sizeof(struct iphdr));
            sendto(sockfd, ethpkt, ETH_FRAME_LEN, 0, (struct sockaddr *)&addr, sizeof(addr));

            curr = head;
            while (curr->next != NULL) curr = curr->next;
            curr->next = malloc(sizeof(struct left_to_receive));
            curr = curr->next;
            curr->id = ntohl(id);
            curr->send_freq = 0;
            curr->last_sent = curr_time;
            memcpy(curr->data, packet, 300);
            curr->next = NULL;
            continue;
        }
        if (FD_ISSET(sockfd, &readfds)) {
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
            if (protocol != 254) continue;
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
            if (msgtype != '1') {
                // printf("Error: Message type is not a response.\n");
                continue;
            }

            // Extracting number of domains from the payload (1 byte after message type)
            char num_domains_char;
            memcpy(&num_domains_char, payload + sizeof(int) + sizeof(char), sizeof(char));

            int num_domains = num_domains_char - '0'; // Converting char to int

            // printf("Number of Domains: %d\n", num_domains);
            // fflush(stdout);

            printf("Query ID: %d\n", id);
            // remove the ID from the linked list
            printf("Total query strings: %d\n", num_domains);
            struct left_to_receive *curr = head->next;
            struct left_to_receive *prev = head;
            while (curr != NULL) {
                if (curr->id == id) {
                    break;
                }
                prev = curr;
                curr = curr->next;
            }
            int offset = sizeof(int) + sizeof(char) + sizeof(char);
            int offset1 = offset;
            for (int i = 0; i < num_domains; i++) {
                char valid;
                memcpy(&valid, payload + offset, sizeof(char));
                offset += sizeof(char);
                unsigned int ip_int;
                memcpy(&ip_int, payload + offset, sizeof(unsigned int));
                struct in_addr ip;
                ip.s_addr = ntohl(ip_int);
                offset += sizeof(unsigned int);

                
                int len;
                memcpy(&len, curr->data + offset1, sizeof(int));
                len = ntohl(len);
                offset1 += sizeof(int);
                char domain[32]; memset(domain, 0, 32);
                memcpy(domain, curr->data + offset1, len);
                offset1 += len;
                printf("%s\t : ", domain);
                
                if (valid == '1') {
                    printf("%s\n", inet_ntoa(ip));

                } else {
                    printf("No IP address found\n", i + 1);
                }
            }
            prev->next = curr->next;
            free(curr);
        }
    }
}