#include "msocket.h"

#define SEM_MACRO 5
#define SHM_MACRO 6
#define SEM_MACRO2 7
#define SHM_MACRO2 8

#define wait(s) semop(s, &pop, 1) 
#define signall(s) semop(s, &vop, 1)

void sighandler (int signum) {
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    shmctl(shm_sockhand, IPC_RMID, NULL);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    semctl(sem_join, 0, IPC_RMID);
    int shm_sock_info = shmget(ftok("msocket.h", SHM_MACRO2), sizeof(struct sock_info), 0777 | IPC_CREAT);
    shmctl(shm_sock_info, IPC_RMID, NULL);
    int sem_soc_create = semget(ftok("msocket.h", SEM_MACRO2), 3, 0777 | IPC_CREAT);
    semctl(sem_soc_create, 0, IPC_RMID);
    exit(0);
}

// ???? signal handling to close all open sockets

int sendACK(int sockfd, int lastInorderSeqNum, int windowSize, int index) {
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SM[index].dest_port);
    addr.sin_addr.s_addr = inet_addr(SM[index].dest_ip_addr);

    char ackBuffer[7];

    ackBuffer[0]=(char)('0'+ 0);
    ackBuffer[1]='A'; ackBuffer[2]='C'; ackBuffer[3]='K'; ackBuffer[6]='\0';
    ackBuffer[4] = (char)('0' + lastInorderSeqNum);
    ackBuffer[5] = (char)('0' + windowSize);
    int len = sendto(sockfd, ackBuffer, 7, 0, (struct sockaddr*)&addr, sizeof(addr));
    return len;
}
// ???? is there a limit to the socket calls or the bind calls
// ???? how to check for dest ip and port if we are not sending

void* R() {
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    while (1){
        fd_set fds;
        FD_ZERO(&fds);
        // add all the fds in socket_id of SM
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            if (SM[i].is_alloted != 0) FD_SET(SM[i].socket_id, &fds);
            signall(sem_join);
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
                char buf[MAXBUF]; memset(buf, 0, MAXBUF);
                // ???? checking if this is same as the dest IP and port
                int n = recvfrom(SM[i].socket_id, buf, MAXBUF, 0, (struct sockaddr*)&addr, &len);
                #ifdef DEBUG
                printf("got recieved %s \n", buf);
                fflush(stdout);
                #endif
                if (n == -1 /*|| dropMessage(p) == 1*/) {
                    signall(sem_join);
                    pop.sem_num = vop.sem_num = 0;
                    continue;
                }
                int msg_seq_no = MAXSEQNO;
                msg_seq_no = (int)(buf[0]-'0');
                if (msg_seq_no > 0 && msg_seq_no < MAXSEQNO) {    
                    int base_seq = SM[i].recv_seq_no;
                    int iter = SM[i].rwnd_markers[0];
                    int flag = 0;
                    while ( iter != (SM[i].rwnd_markers[1]+1)%RWND) {
                        if (msg_seq_no == base_seq) {
                            if (iter == SM[i].rwnd_markers[0]) {
                                strcpy(SM[i].recv_buf[iter], buf+1);
                                SM[i].recv_status[iter] = msg_seq_no;
                                #ifdef DEBUG
                                printf ("setting SM[%d].recv_buf[%d] to %s\n", i, iter, SM[i].recv_buf[iter]);
                                fflush(stdout);
                                #endif
                                while (SM[i].recv_status[iter] != 0 && iter != (SM[i].rwnd_markers[1]+1)%RWND) { 
                                    SM[i].rwnd_markers[0] = (SM[i].rwnd_markers[0]+1)%RWND;
                                    if (SM[i].recv_status[(SM[i].rwnd_markers[1]+1)%RWND] == 0) SM[i].rwnd_markers[1] = (SM[i].rwnd_markers[1]+1)%RWND;
                                    else SM[i].rwnd.size -= 1;
                                    SM[i].recv_seq_no = (SM[i].recv_seq_no+1)%MAXSEQNO;
                                    iter = (iter+1)%RWND;
                                }
                                int len = sendACK(SM[i].socket_id, (SM[i].recv_seq_no-1), SM[i].rwnd.size, i);
                                flag = 1;
                            }
                            else {
                                strcpy(SM[i].recv_buf[iter], buf+1);
                                SM[i].recv_status[iter] = msg_seq_no;
                                flag = 1;
                            }
                        }                        
                        iter = (iter+1)%RWND;
                        base_seq = (base_seq+1)%MAXSEQNO;
                        if (base_seq == 0) base_seq = 1;
                    }
                    if (flag == 0) { int len = sendACK(SM[i].socket_id, SM[i].recv_seq_no, SM[i].rwnd.size, i); }
                }
                else if (msg_seq_no == 0) {
                    int ack_value;
                    char ack_type[4];
                    int window_size;
                    int last_inorder_seq_num;
                    ack_type[0] = buf[1]; ack_type[1] = buf[2]; ack_type[2] = buf[3]; ack_type[3] = buf[4];
                    last_inorder_seq_num = (int)(buf[5]-'0');
                    window_size = (int)(buf[6]-'0');
                    
                    int iter = SM[i].swnd_markers[0]; int j = 0;
                    while (j < SM[i].swnd.size) {
                        if (SM[i].swnd.seq_no[iter] == last_inorder_seq_num) break;
                        iter = (iter+1)%SWND; j++;
                    }
                    if (j == SM[i].swnd.size) {
                        signall(sem_join);
                        pop.sem_num = vop.sem_num = 0;
                        continue;
                    }
                    int iter_upto = (iter+1)%SWND;
                    iter = SM[i].swnd_markers[0]; int k = 0;
                    while (k <= j) {
                        memset(SM[i].send_buf[iter], 0, MAXBLOCK);
                        // SM[i].send_status[iter] = 0;
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
            signall(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        

    }
    pthread_exit(NULL);
}

void* S(){
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    while (1) {
        // #ifdef DEBUG
        // printf("in the S function\n"); fflush(stdout);
        // #endif
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            struct timeval curr_time;
            gettimeofday(&curr_time, NULL);
            if (SM[i].is_alloted == 0) {
                signall(sem_join);
                pop.sem_num = vop.sem_num = 0;
                continue;
            }
            if (SM[i].send_time[SM[i].swnd_markers[0]].tv_usec != 0 && curr_time.tv_usec - SM[i].send_time[SM[i].swnd_markers[0]].tv_usec > (((int)T) * 1000000)) {
                int iter = SM[i].swnd_markers[0]; int j = 0;
                while (j < SM[i].swnd.size) {
                    if (/*SM[i].send_status[iter] == 1 && */SM[i].is_alloted == 1 && SM[i].swnd.seq_no[iter] != -1) {
                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(SM[i].dest_port);
                        addr.sin_addr.s_addr = inet_addr(SM[i].dest_ip_addr);
                        int len = sizeof(addr);
                        char temp_buf[MAXBUF]; memset(temp_buf, 0, MAXBUF);
                        temp_buf[0] = (char)(SM[i].swnd.seq_no[iter]+'0');
                        strcpy(temp_buf+1, SM[i].send_buf[iter]);
                        #ifdef DEBUG
                        printf("the seq no. sent again : %d and msg : %s\n", SM[i].swnd.seq_no[iter], temp_buf);
                        fflush(stdout);
                        #endif
                        int n = sendto(SM[i].socket_id, temp_buf, strlen(SM[i].send_buf[iter])+1, 0, (struct sockaddr*)&addr, len);
                        if (n == -1) {
                            continue;
                        }
                        gettimeofday(&curr_time, NULL);
                        SM[i].send_time[iter] = curr_time;
                    }
                    iter = (iter+1)%SWND;
                    j++;
                }
            }
            else {
                int iter = SM[i].swnd_markers[0]; int j = 0;
                // #ifdef DEBUG
                // printf("no timeout and initial send marker : %d and final send marker : %d\n", SM[i].swnd_markers[0], SM[i].swnd_markers[1]);
                // fflush(stdout);
                // #endif
                while (j < SM[i].swnd.size) {
                    // #ifdef DEBUG
                    // printf("i : %d send status : %d and send time : %d and is alloted : %d and seq no : %d\n",i, SM[i].send_status[iter], SM[i].send_time[iter].tv_usec, SM[i].is_alloted, SM[i].swnd.seq_no[iter]);
                    // fflush(stdout);
                    // #endif
                    if (/*SM[i].send_status[iter] == 1 && */SM[i].send_time[iter].tv_usec == 0 && SM[i].is_alloted == 1 && SM[i].swnd.seq_no[iter] != -1) {
                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(SM[i].dest_port);
                        addr.sin_addr.s_addr = inet_addr(SM[i].dest_ip_addr);
                        #ifdef DEBUG
                        printf("dest ip and port %s %d with socket %d\n", SM[i].dest_ip_addr, SM[i].dest_port, SM[i].socket_id);
                        fflush(stdout);
                        #endif
                        int len = sizeof(addr);
                        char temp_buf[MAXBUF]; memset(temp_buf, 0, MAXBUF);
                        temp_buf[0] = (char)(SM[i].swnd.seq_no[iter]+'0');
                        strcpy(temp_buf+1, SM[i].send_buf[iter]);
                        int n = sendto(SM[i].socket_id, temp_buf, strlen(SM[i].send_buf[iter])+1, 0, (const struct sockaddr*)&addr, len);
                        if (n == -1) {
                            iter = (iter+1)%SWND;
                            continue;
                        }
                        gettimeofday(&curr_time, NULL);
                        SM[i].send_time[iter].tv_usec = curr_time.tv_usec;
                        #ifdef DEBUG
                        printf("the seq. no. sent : %d and msg : %s at time: %d\n", SM[i].swnd.seq_no[iter], temp_buf, SM[i].send_time[iter].tv_usec);
                        fflush(stdout);
                        #endif
                    }
                    iter = (iter+1)%SWND;
                    j++;
                }
            }
            signall(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        sleep((int)T/2);
    }
    pthread_exit(NULL);
}

void* G(){

    pthread_exit(NULL);
}



int main() {
    int shm_sockhand;
    shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    if(shm_sockhand == -1) {
        perror("shmget");
        exit(1);
    }
    
    int shm_sock_info = shmget(ftok("msocket.h", SHM_MACRO2), sizeof(struct sock_info), 0777 | IPC_CREAT);
    struct sock_info* sock_info = (struct sock_info*)shmat(shm_sock_info, NULL, 0);
    if(shm_sock_info == -1) {
        perror("shmget");
        exit(1);
    }
    // set all the values of sock_info to 0
    sock_info->sockfd = 0;
    memset(sock_info->src_ip, 0, MAXIP);
    sock_info->src_port = 0;

    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    if (sem_join == -1) {
        perror("semget");
        return 1;
    }

    int sem_soc_create = semget(ftok("msocket.h", SEM_MACRO2), 3, 0777 | IPC_CREAT);
    if (sem_soc_create == -1) {
        perror("semget");
        return 1;
    }
    for (int i = 0; i < 2; i++) {
        if (semctl(sem_soc_create, i, SETVAL, 0) == -1) {
            perror("semctl");
            return 1;
        }
    }
    if (semctl(sem_soc_create, 2, SETVAL, 1)) {
        perror("semctl");
        return 1;
    }

    for (int i = 0; i < MAXSOCKETS; i++) {
        if (semctl(sem_join, i, SETVAL, 1) == -1) {
            perror("semctl");
            return 1;
        }
    }

    for(int i=0; i<MAXSOCKETS; i++){
        SM[i].is_alloted = 0;
        memset(SM[i].src_ip_addr, 0, MAXIP);
        memset(SM[i].dest_ip_addr, 0, MAXIP);
        memset(SM[i].send_buf, 0, SWND*(MAXBLOCK));
        memset(SM[i].recv_buf, 0, RWND*(MAXBLOCK));
        SM[i].send_seq_no = 1;
        SM[i].recv_seq_no = 1;
        int n = (int)MAXWNDW-1; int m = ((int)MAXSEQNO)/2;
        SM[i].swnd_markers[0] = 0; SM[i].swnd_markers[1] = n > m ? m : n;
        SM[i].rwnd_markers[0] = 0; SM[i].rwnd_markers[1] = n > m ? m : n;
        for (int j=0; j<(RWND > SWND ? RWND : SWND); j++){
            SM[i].rwnd.seq_no[j] = -1;
            SM[i].swnd.seq_no[j] = -1;
            if (j < SWND ) { /*SM[i].send_status[j] = 0; */SM[i].send_time[j].tv_sec = 0; SM[i].send_time[j].tv_usec = 0; }
            if (j < RWND ) SM[i].recv_status[j] = 0;
        }
        SM[i].rwnd.size = MAXWNDW;
        SM[i].swnd.size = MAXWNDW;
        SM[i].rwnd_markers[2] = 1;
    }
    pthread_t Rid, Sid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&Rid, &attr, R, NULL);
    pthread_create(&Sid, &attr, S, NULL);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    while (1) {
        pop.sem_num = vop.sem_num = 0;
        wait(sem_soc_create);
        if (sock_info->sockfd == 0 && sock_info->src_port == 0){
            // create a socket for the particular sockfd
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockfd == -1) {
                perror("socket");
                sock_info->err = 1;
                pop.sem_num = vop.sem_num = 0;
                signall(sem_soc_create);
                continue;
            }
            sock_info->sockfd = sockfd;
            pop.sem_num = vop.sem_num = 1;
            signall(sem_soc_create);
        } else if (sock_info->sockfd != 0 && sock_info->src_port >= 0) {
            // bind the socket to the particular port
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(sock_info->src_port);
            addr.sin_addr.s_addr = inet_addr(sock_info->src_ip);
            if (bind(sock_info->sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                perror("bind");
                sock_info->err = 1;
                pop.sem_num = vop.sem_num = 1;
                signall(sem_soc_create);
                continue;
            }
            sock_info->err = 0;
            pop.sem_num = vop.sem_num = 1;
            signall(sem_soc_create);
            continue;
        } else if (sock_info->src_port == -1) {
            // close the socket
            int m = close(sock_info->sockfd);
            if (m == -1) {
                perror("close");
                sock_info->err = 1;
                pop.sem_num = vop.sem_num = 1;
                signall(sem_soc_create);
                continue;
            }
            pop.sem_num = vop.sem_num = 1;
            signall(sem_soc_create);
        }
    }
    
    pthread_join(Rid, NULL);
    pthread_join(Sid, NULL);

    shmctl(shm_sock_info, IPC_RMID, NULL);
    shmctl(shm_sockhand, IPC_RMID, NULL);
    semctl(sem_soc_create, 0, IPC_RMID);
    semctl(sem_join, 0, IPC_RMID);
    return 0;
}
