#include "msocket.h"

#define SEM_MACRO 5
#define SHM_MACRO 6
#define SEM_MACRO2 7
#define SHM_MACRO2 8

#define wait(s) semop(s, &pop, 1) 
#define signall(s) semop(s, &vop, 1)


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
        int max_fd = 0;
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            if (SM[i].is_alloted != 0) FD_SET(SM[i].socket_id, &fds);
            if (SM[i].is_alloted != 0 && SM[i].socket_id > max_fd) max_fd = SM[i].socket_id;
            signall(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        struct timeval m_timer;
        m_timer.tv_sec = T;
        m_timer.tv_usec = 0;
        select(max_fd+1, &fds, NULL, NULL, &m_timer);
        
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            if (SM[i].is_alloted != 0 && m_timer.tv_sec == 0) {
                if (SM[i].flag_nospace == 1 && SM[i].rwnd.size > 0){
                    int recv_send_ack = SM[i].recv_seq_no-1;
                    if (recv_send_ack == 0) recv_send_ack = MAXSEQNO-1;
                    int len = sendACK(SM[i].socket_id, recv_send_ack, SM[i].rwnd.size, i);
                }
                signall(sem_join);
                pop.sem_num = vop.sem_num = 0;
                continue;
            }
            if (SM[i].is_alloted != 0 && FD_ISSET(SM[i].socket_id, &fds)) {
                struct sockaddr_in addr;
                int len = sizeof(addr);
                char buf[MAXBUF]; memset(buf, 0, MAXBUF);
                int n = recvfrom(SM[i].socket_id, buf, MAXBUF, 0, (struct sockaddr*)&addr, &len);
                if (n == -1 || dropMessage(p) == 1) {
                    #ifdef DEBUG
                    printf("dropping : ");
                    for (int m = 0; m < n; m++) printf("%c", buf[m]);
                    printf("\n");
                    fflush(stdout);
                    #endif
                    signall(sem_join);
                    pop.sem_num = vop.sem_num = 0;
                    continue;
                }
                int msg_seq_no = MAXSEQNO;
                msg_seq_no = (int)(buf[0]-'0');
                if (msg_seq_no > 0 && msg_seq_no < MAXSEQNO) {
                    if (SM[i].flag_nospace == 1)  SM[i].flag_nospace = 0;    
                    int base_seq = SM[i].recv_seq_no;
                    int iter = SM[i].rwnd_markers[0];
                    int flag = 0; int j = 0;
                    while ( j < SM[i].rwnd.size && iter < RWND) {
                        if (msg_seq_no == base_seq) {
                            if (iter == SM[i].rwnd_markers[0]) {
                                memcpy(SM[i].recv_buf[iter], buf+1, n-1);
                                SM[i].recv_len[iter] = n-1;
                                SM[i].recv_status[iter] = msg_seq_no;
                                #ifdef DEBUG
                                printf ("setting in order SM[%d].recv_buf[%d] ", i, iter);
                                printf("got recieved : ");
                                for (int m = 0; m < n; m++) printf("%c", buf[m]);
                                printf("\n");
                                printf("send seq no now: %d\n", SM[i].send_seq_no);
                                fflush(stdout);
                                #endif
                                #ifdef DEBUG
                                printf("updating recv window at i: %d from (%d,%d) ",i, SM[i].rwnd_markers[0],SM[i].rwnd_markers[1]);
                                fflush(stdout);
                                #endif
                                while (j < SM[i].rwnd.size && iter < RWND && SM[i].recv_status[iter] != 0) { 
                                    SM[i].rwnd_markers[0] = (SM[i].rwnd_markers[0]+1)%RWND;
                                    if (SM[i].recv_status[(SM[i].rwnd_markers[1]+1)%RWND] == 0) SM[i].rwnd_markers[1] = (SM[i].rwnd_markers[1]+1)%RWND;
                                    else SM[i].rwnd.size -= 1;
                                    SM[i].recv_seq_no = (SM[i].recv_seq_no+1)%MAXSEQNO;
                                    if (SM[i].recv_seq_no == 0) SM[i].recv_seq_no = 1;
                                    iter = (iter+1)%RWND;
                                    j++;
                                }  
                                #ifdef DEBUG
                                printf("to (%d,%d) and recv_seq : %d\n",SM[i].rwnd_markers[0],SM[i].rwnd_markers[1], SM[i].recv_seq_no);
                                fflush(stdout);
                                #endif
                                if (SM[i].rwnd.size == 0) SM[i].flag_nospace = 1;
                                int recv_send_ack = SM[i].recv_seq_no-1;
                                if (recv_send_ack == 0) recv_send_ack = MAXSEQNO-1;
                                int len = sendACK(SM[i].socket_id, recv_send_ack, SM[i].rwnd.size, i);
                                flag = 1;
                                break;
                            }
                            else {
                                memcpy(SM[i].recv_buf[iter],buf+1,n-1);
                                SM[i].recv_len[iter] = n-1;
                                #ifdef DEBUG
                                    printf ("setting SM[%d].recv_buf[%d] ", i, iter);
                                    printf("got recieved : ");
                                    for (int m = 0; m < n; m++) printf("%c", buf[m]);
                                    printf("\n");
                                    printf("send seq no now: %d\n", SM[i].send_seq_no);
                                    fflush(stdout);
                                #endif
                                SM[i].recv_status[iter] = msg_seq_no;
                                flag = 1;
                                break;
                            }
                        }                   
                        iter = (iter+1)%RWND;
                        j++;
                        base_seq = (base_seq+1)%MAXSEQNO;
                        if (base_seq == 0) base_seq = 1;
                    }
                    if (flag == 0) { 
                        int recv_send_ack = SM[i].recv_seq_no-1;
                        if (recv_send_ack == 0) recv_send_ack = MAXSEQNO-1;
                        int len = sendACK(SM[i].socket_id, recv_send_ack, SM[i].rwnd.size, i); 
                    }
                }
                else if (msg_seq_no == 0) {
                    #ifdef DEBUG
                    printf("got recieved : ");
                    for (int m = 0; m < n; m++) printf("%c", buf[m]);
                    printf("\n");
                    printf("send seq no now: %d\n", SM[i].send_seq_no);
                    fflush(stdout);
                    #endif
                    int ack_value;
                    char ack_type[4];
                    int window_size;
                    int last_inorder_seq_num;
                    ack_type[0] = buf[1]; ack_type[1] = buf[2]; ack_type[2] = buf[3]; ack_type[3] = buf[6];
                    last_inorder_seq_num = (int)(buf[4]-'0');
                    window_size = (int)(buf[5]-'0');
                    int iter = SM[i].swnd_markers[0]; int j = 0;
                    if (SM[i].swnd.size == 0) {
                        // int hops = 0; int my_iter = last_inorder_seq_num;
                        // while (my_iter != SM[i].swnd.seq_no[SM[i].swnd_markers[0]]) {
                        //     my_iter = (my_iter+1)%MAXSEQNO;
                        //     hops++;
                        // }
                        // int hops2 = 0;
                        // my_iter = last_inorder_seq_num;
                        // while (my_iter != SM[i].send_seq_no) {
                        //     my_iter = (my_iter+1)%MAXSEQNO;
                        //     hops2++;
                        // }
                        if (window_size > 0) {
                            #ifdef DEBUG
                            printf("updating send wdw after nospace at i: %d from (%d,%d) to ",i, SM[i].swnd_markers[0],SM[i].swnd_markers[1]);
                            fflush(stdout);
                            #endif
                            SM[i].swnd_markers[1] = (SM[i].swnd_markers[0]+window_size-1)%SWND;
                            SM[i].swnd.size = window_size;
                            #ifdef DEBUG
                            printf("(%d,%d)\n",SM[i].swnd_markers[0],SM[i].swnd_markers[1]);
                            fflush(stdout);
                            #endif
                        }
                        signall(sem_join);
                        pop.sem_num = vop.sem_num = 0;
                        continue;
                    }
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
                    while (k <= j && iter < SWND) {
                        memset(SM[i].send_buf[iter], 0, MAXBLOCK);
                        SM[i].send_len[iter] = 0;
                        SM[i].send_time[iter].tv_sec = 0;
                        SM[i].send_time[iter].tv_usec = 0;
                        SM[i].swnd.seq_no[iter] = -1;
                        #ifdef DEBUG
                        printf("setting SM[%d].swnd.seq_no[%d] to -1\n", i, iter);
                        fflush(stdout);
                        #endif
                        iter = (iter+1)%SWND;
                        k++;
                    }
                    #ifdef DEBUG
                    printf("updating send window at i : %d from (%d,%d) to ",i, SM[i].swnd_markers[0],SM[i].swnd_markers[1]);
                    fflush(stdout);
                    #endif
                    SM[i].swnd_markers[0] = iter_upto;
                    if (window_size != 0) SM[i].swnd_markers[1] = (SM[i].swnd_markers[0]+window_size-1)%SWND;
                    SM[i].swnd.size = window_size;
                    #ifdef DEBUG
                    printf("(%d,%d) in i: %d\n",SM[i].swnd_markers[0],SM[i].swnd_markers[1],i);
                    fflush(stdout);
                    #endif
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
            long int duration = ((curr_time.tv_sec - SM[i].send_time[SM[i].swnd_markers[0]].tv_sec) * 1000000 + curr_time.tv_usec - SM[i].send_time[SM[i].swnd_markers[0]].tv_usec)/1000000;
            if ((SM[i].send_time[SM[i].swnd_markers[0]].tv_sec != 0 || SM[i].send_time[SM[i].swnd_markers[0]].tv_usec != 0) && duration > (int)T) {
                int iter = SM[i].swnd_markers[0]; int j = 0;
                while (j < SM[i].swnd.size && iter < SWND) {
                    if (/*SM[i].send_status[iter] == 1 && */SM[i].is_alloted == 1 && SM[i].swnd.seq_no[iter] != -1) {
                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(SM[i].dest_port);
                        addr.sin_addr.s_addr = inet_addr(SM[i].dest_ip_addr);
                        int len = sizeof(addr);
                        char temp_buf[MAXBUF]; memset(temp_buf, 0, MAXBUF);
                        temp_buf[0] = (char)(SM[i].swnd.seq_no[iter]+'0');
                        memcpy(temp_buf+1, SM[i].send_buf[iter], SM[i].send_len[iter]);
                        #ifdef DEBUG
                        printf("the seq no. sent again : %d and msg : ", SM[i].swnd.seq_no[iter]);
                        for (int m = 0; m <= SM[i].send_len[iter]; m++) printf("%c", temp_buf[m]);
                        printf("\n");
                        fflush(stdout);
                        #endif
                        int n = sendto(SM[i].socket_id, temp_buf, SM[i].send_len[iter]+1, 0, (struct sockaddr*)&addr, len);
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
                while (j < SM[i].swnd.size && iter < SWND) {
                    if (SM[i].send_time[iter].tv_sec == 0 && SM[i].send_time[iter].tv_usec == 0 && SM[i].is_alloted == 1 && SM[i].swnd.seq_no[iter] != -1) {
                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(SM[i].dest_port);
                        addr.sin_addr.s_addr = inet_addr(SM[i].dest_ip_addr);
                        int len = sizeof(addr);
                        char temp_buf[MAXBUF]; memset(temp_buf, 0, MAXBUF);
                        temp_buf[0] = (char)(SM[i].swnd.seq_no[iter]+'0');
                        memcpy(temp_buf+1, SM[i].send_buf[iter], SM[i].send_len[iter]);
                        int n = sendto(SM[i].socket_id, temp_buf, SM[i].send_len[iter]+1, 0, (const struct sockaddr*)&addr, len);
                        if (n == -1) {
                            iter = (iter+1)%SWND;
                            continue;
                        }
                        gettimeofday(&curr_time, NULL);
                        SM[i].send_time[iter] = curr_time;
                        #ifdef DEBUG
                        printf("the seq no. sent : %d and msg : ", SM[i].swnd.seq_no[iter]);
                        for (int m = 0; m <= SM[i].send_len[iter]; m++) printf("%c", temp_buf[m]);
                        printf(" at time : %ld\n", SM[i].send_time[iter].tv_sec);
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
    signal(SIGINT, sighandler);
    // timer would be much larger than the S thread timeout time
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    while (1) {
        for(int i=0; i<MAXSOCKETS; i++) {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            if (SM[i].is_alloted == 0) {
                signall(sem_join);
                pop.sem_num = vop.sem_num = 0;
                continue;
            }
            // check if the process assc with the socket is still running
            int status;
            int check = waitpid(SM[i].process_id, &status, WNOHANG);
            if (check == 0) {
                signall(sem_join);
                pop.sem_num = vop.sem_num = 0;
                continue;
            }
            // if the process is not running then close the socket
            // #ifdef DEBUG
            // printf("checking at i: %d\n", i);
            // fflush(stdout);
            // #endif
            struct stat sts; char path_to_process[1000]; memset(path_to_process, 0, 1000);
            sprintf(path_to_process, "/proc/%d", SM[i].process_id);
            if (stat(path_to_process, &sts) == -1 && errno == ENOENT) {
                #ifdef DEBUG
                printf("closing socket %d at i: %d\n", SM[i].socket_id, i);
                fflush(stdout);
                #endif
                close(SM[i].socket_id);
                SM[i].is_alloted = 0;
                memset(SM[i].src_ip_addr, 0, MAXIP);
                memset(SM[i].dest_ip_addr, 0, MAXIP);
                memset(SM[i].send_buf, 0, SWND*(MAXBLOCK));
                for (int j=0; j<SWND; j++) SM[i].send_len[j] = 0;
                memset(SM[i].recv_buf, 0, RWND*(MAXBLOCK));
                for (int j=0; j<RWND; j++) SM[i].recv_len[j] = 0;
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
                SM[i].flag_nospace = 0;
            }            
            signall(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        sleep((int)2*T);
    }
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
        for (int j=0; j<SWND; j++) SM[i].send_len[j] = 0;
        memset(SM[i].recv_buf, 0, RWND*(MAXBLOCK));
        for (int j=0; j<RWND; j++) SM[i].recv_len[j] = 0;
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
    pthread_t Rid, Sid, Gid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&Rid, &attr, R, NULL);
    pthread_create(&Sid, &attr, S, NULL);
    pthread_create(&Gid, &attr, G, NULL);
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
                // perror("socket");
                sock_info->err = errno;
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
                // perror("bind");
                sock_info->err = errno;
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
                // perror("close");
                sock_info->err = errno;
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
