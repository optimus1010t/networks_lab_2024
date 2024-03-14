#include "msocket.h"

#define wait(s) semop(s, &pop, 1) 
#define signal(s) semop(s, &vop, 1) 


int sendACK(int sockfd, int lastInorderSeqNum, int windowSize, int index) {
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SM[index].dest_port);
    addr.sin_addr.s_addr = inet_addr(SM[index].dest_ip_addr);

    int lenofack = sizeof(int) + 4 + sizeof(int) * 2;
    char* ackBuffer = (char*)malloc(lenofack); 
    int g = 0;

    memcpy(ackBuffer, &g, sizeof(int));
    strncpy(ackBuffer +  sizeof(int), "ACK", 4);
    ackBuffer[sizeof(int)+3] = '\0'; // Null terminate the string
    memcpy(ackBuffer + sizeof(int) + 4, &windowSize, sizeof(int)); // Copy the window size to the buffer
    memcpy(ackBuffer + sizeof(int) + 4 + sizeof(int), &lastInorderSeqNum, sizeof(int)); // Copy the last in-order sequence number to the buffer
    int len = sendto(sockfd, ackBuffer, lenofack, 0, (struct sockaddr*)&addr, sizeof(addr));
    free(ackBuffer);
    return len;
}
// ???? is there a limit ot the socket calls or the bind calls
// ???? use semaphores to protect the shared memory
// ???? how to check for dest ip and port if we are not sending 

void* R(){
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    while (1){
        fd_set fds;
        FD_ZERO(&fds);
        // add all the fds in socket_id of SM
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            if (SM[i].is_alloted != 0) FD_SET(SM[i].socket_id, &fds);
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        struct timeval m_timer;
        m_timer.tv_sec = T;
        m_timer.tv_usec = 0;
        select(FD_SETSIZE, &fds, NULL, NULL, &m_timer);
        if (m_timer.tv_sec == 0) continue;
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            if (SM[i].is_alloted != 0 && FD_ISSET(SM[i].socket_id, &fds)) {
                struct sockaddr_in addr;
                int len = sizeof(addr);
                char buf[MAXBUF];
                // ???? checking if this is same as the dest IP and port
                int n = recvfrom(SM[i].socket_id, buf, MAXBUF, 0, (struct sockaddr*)&addr, &len);
                if (n == -1 || dropMessage(p) == 1) {
                    signal(sem_join);
                    pop.sem_num = vop.sem_num = 0;
                    continue;
                }
                int msg_seq_no;
                memcpy(&msg_seq_no, buf, sizeof(int));
                msg_seq_no = ntohl(msg_seq_no);
                size_t msg_len = n-sizeof(int);
                if (msg_seq_no != 0) {    
                    int base_seq = SM[i].recv_seq_no;
                    int iter = SM[i].rwnd_markers[0];
                    int flag = 0;
                    while ( iter != SM[i].rwnd_markers[1]+1 ) {
                        if (SM[i].recv_status[iter] == 0) {
                            strcpy(SM[i].recv_buf[iter], buf+sizeof(int));
                            SM[i].recv_status[iter] = 1;
                            if (msg_seq_no == base_seq  && iter == SM[i].rwnd_markers[0]) {
                                SM[i].rwnd_markers[0] = (SM[i].rwnd_markers[0]+1)%RWND;
                                if (SM[i].recv_status[(SM[i].rwnd_markers[1]+1)%RWND] == 0) SM[i].rwnd_markers[1] = (SM[i].rwnd_markers[1]+1)%RWND;
                                else SM[i].rwnd.size -= 1;
                                int len = sendACK(SM[i].socket_id, SM[i].recv_seq_no, SM[i].rwnd.size, i);
                            }
                            flag = 1;
                            SM[i].recv_seq_no = (SM[i].recv_seq_no+1)%MAXSEQNO;
                            break;
                        }
                        iter = (iter+1)%RWND;
                        base_seq = (base_seq+1)%MAXSEQNO;
                        if (base_seq == 0) base_seq = 1;
                    }
                    if (flag == 0) {
                        int len = sendACK(SM[i].socket_id, SM[i].recv_seq_no, SM[i].rwnd.size, i);
                    }
                }
                else {
                    int ack_value;
                    char ack_type[4];
                    int window_size;
                    int last_inorder_seq_num;

                    memcpy(&ack_value, buf, sizeof(int));
                    strncpy(ack_type, buf + sizeof(int), 4);
                    ack_type[3] = '\0'; // Null terminate the string
                    memcpy(&window_size, buf + sizeof(int) + 4, sizeof(int));
                    memcpy(&last_inorder_seq_num, buf + sizeof(int) + 4 + sizeof(int), sizeof(int));
                    
                    int iter = SM[i].swnd_markers[0];
                    while (iter != (SM[i].swnd_markers[1]+1)%SWND) {
                        if (SM[i].swnd.seq_no[iter] == last_inorder_seq_num) break;
                        iter = (iter+1)%SWND;
                    }
                    if (iter == (SM[i].swnd_markers[1]+1)%SWND) {
                        signal(sem_join);
                        pop.sem_num = vop.sem_num = 0;
                        continue;
                    }
                    int iter_upto = (iter+1)%SWND;
                    iter = SM[i].swnd_markers[0];
                    while (iter != iter_upto) {
                        memset(SM[i].send_buf[iter], 0, MAXBLOCK);
                        SM[i].send_status[iter] = 0;
                        SM[i].send_time[iter].tv_sec = 0;
                        SM[i].send_time[iter].tv_usec = 0;
                        SM[i].swnd.seq_no[iter] = -1;
                        iter = (iter+1)%SWND;
                    }
                    SM[i].swnd_markers[0] = iter_upto;
                    SM[i].swnd_markers[1] = (SM[i].swnd_markers[0]+window_size)%SWND;
                    SM[i].swnd.size = window_size;
                }                
            }
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        

    }
    pthread_exit(NULL);
}

void* S(){
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    while (1) {
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            struct timeval curr_time;
            gettimeofday(&curr_time, NULL);
            if (SM[i].send_time[SM[i].swnd_markers[0]].tv_sec != 0 && curr_time.tv_sec - SM[i].send_time[SM[i].swnd_markers[0]].tv_sec > T) {
                int iter = SM[i].swnd_markers[0];
                while (iter != (SM[i].swnd_markers[1]+1)%SWND) {
                    if (SM[i].send_status[iter] == 1 && SM[i].is_alloted == 1 && SM[i].swnd.seq_no[iter] != -1) {
                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(SM[i].dest_port);
                        addr.sin_addr.s_addr = inet_addr(SM[i].dest_ip_addr);
                        int len = sizeof(addr);
                        // add sequence number to the message and get the sequence number from the array ????
                        int n = sendto(SM[i].socket_id, SM[i].send_buf[iter], strlen(SM[i].send_buf[iter]), 0, (struct sockaddr*)&addr, len);
                        if (n == -1) {
                            continue;
                        }
                        SM[i].send_time[iter] = curr_time;
                    }
                    iter = (iter+1)%SWND;
                }
            }
            else {
                int iter = SM[i].swnd_markers[0];
                while (iter != (SM[i].swnd_markers[1]+1)%SWND) {
                    if (SM[i].send_status[iter] == 1 && SM[i].is_alloted == 1) {
                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(SM[i].dest_port);
                        addr.sin_addr.s_addr = inet_addr(SM[i].dest_ip_addr);
                        int len = sizeof(addr);
                        // add sequence number to the message ????
                        int n = sendto(SM[i].socket_id, SM[i].send_buf[iter], strlen(SM[i].send_buf[iter]), 0, (struct sockaddr*)&addr, len);
                        if (n == -1) {
                            continue;
                        }
                        SM[i].send_time[iter] = curr_time;
                        SM[i].swnd.seq_no[iter] = SM[i].send_seq_no;
                        SM[i].send_seq_no = (SM[i].send_seq_no+1)%MAXSEQNO;
                    }
                    iter = (iter+1)%SWND;
                }
            }
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        sleep(T/2);
    }
    pthread_exit(NULL);
}

void* G(){
    pthread_exit(NULL);
}

int m_recvfrom (int sockfd, void *buf, size_t len, int flags) {
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    pop.sem_num = vop.sem_num = sockfd;
    wait(sem_join);
    if (SM[sockfd].is_alloted == 1) {
        int j;
        for (j=0; j<RWND; j++){ 
            if (SM[sockfd].rwnd.seq_no[j] == SM[sockfd].recv_seq_no) break;
        }
        if (j < RWND && SM[sockfd].recv_status[j] == 1) {
            strcpy((char*)buf, SM[sockfd].recv_buf[j]);
            SM[sockfd].recv_status[j] = 0;
            SM[sockfd].recv_seq_no = (SM[sockfd].recv_seq_no+1)%MAXSEQNO;
            if (SM[sockfd].recv_seq_no == 0) SM[sockfd].recv_seq_no = 1;
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return strlen(buf); // ???? what to do if the len is less than the actual len (strlen(buf))
        }
        else {
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return -1;
        }
    }
    signal(sem_join);
    pop.sem_num = vop.sem_num = 0;
    return -1;
}

int m_sendto(int sockfd, const void *buf, size_t len, int flags) {  // ???? what checks for dest ip and port
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    pop.sem_num = vop.sem_num = sockfd;
    wait(sem_join);
    if (SM[sockfd].is_alloted == 1) {
        if (SM[sockfd].send_status[(SM[sockfd].swnd_markers[1]+1)%SWND] == 0) {  
            strcpy(SM[sockfd].send_buf[(SM[sockfd].swnd_markers[1]+1)%SWND], (char*)buf); // ???? just doing for string bufs for now
            SM[sockfd].swnd.seq_no[(SM[sockfd].swnd_markers[1]+1)%SWND] = SM[sockfd].send_seq_no;

            SM[sockfd].send_seq_no = (SM[sockfd].send_seq_no+1)%MAXSEQNO;
            if (SM[sockfd].send_seq_no == 0) SM[sockfd].send_seq_no = 1;
            
            SM[sockfd].send_status[(SM[sockfd].swnd_markers[1]+1)%SWND] = 1;
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return strlen(buf);
        }
        else {
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return -1;
        }
    }else {
        signal(sem_join);
        pop.sem_num = vop.sem_num = 0;
        return -1;
    }
}

int m_socket(int domain, int type, int protocol){ 
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    if (type != SOCK_MTP) return -1;
    if (domain != AF_INET) return -1;
    // if (protocol != 0) return -1; // ???? do we need to check this
    for(int i=0; i<MAXSOCKETS; i++){
        pop.sem_num = vop.sem_num = i;
        wait(sem_join);
        if (SM[i].is_alloted == 0){
            SM[i].is_alloted = 1;
            SM[i].socket_id = socket(AF_INET, SOCK_DGRAM, 0);
            if (SM[i].socket_id == -1) {
                signal(sem_join);
                pop.sem_num = vop.sem_num = 0;
                return -1;
            }
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return i;
        }
        signal(sem_join);
        pop.sem_num = vop.sem_num = 0;
    }
    return -1;    
}

int m_bind(int sockfd, char source_ip[MAXIP], unsigned short int source_port, char dest_ip[MAXIP], unsigned short int dest_port){  
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    
    pop.sem_num = vop.sem_num = sockfd; 
    wait(sem_join);
    if (SM[sockfd].is_alloted == 1){
        strcpy(SM[sockfd].src_ip_addr, source_ip);
        SM[sockfd].src_port = source_port;
        strcpy(SM[sockfd].dest_ip_addr, dest_ip);
        SM[sockfd].dest_port = dest_port;
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((unsigned short)source_port);
        addr.sin_addr.s_addr = inet_addr(source_ip);
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return -1;
        }
        else return 0;
    }
    signal(sem_join);
    pop.sem_num = vop.sem_num = 0;
    return -1;
}

int m_close(int fd){ 
    int sem_join = semget(ftok("msocket.h", 7), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    for (int i=0; i<MAXSOCKETS; i++){
        pop.sem_num = vop.sem_num = i;
        wait(sem_join);
        if (SM[i].is_alloted == 1 && SM[i].socket_id == fd) {
            SM[i].is_alloted = 0;
            close(SM[i].socket_id);
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return 0;
        }else{
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return -1;
        } 
    }
}

int dropMessage(float pp) { 
    // generate a random number between 0 and 1
    float m_num = (float)rand()/(float)(RAND_MAX);
    if (m_num < pp) {
        return 1;
    }
    return 0;
}
