#include "msocket.h"

#define wait(s) semop(s, &pop, 1) 
#define signall(s) semop(s, &vop, 1) 

#define SEM_MACRO 5
#define SHM_MACRO 6
#define SEM_MACRO2 7
#define SHM_MACRO2 8

void sighandler (int signum) {
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    shmctl(shm_sockhand, IPC_RMID, NULL);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    semctl(sem_join, 0, IPC_RMID);
    int sem_soc_create = semget(ftok("msocket.h", SEM_MACRO2), 3, 0777 | IPC_CREAT);
    semctl(sem_soc_create, 0, IPC_RMID);
    int shm_sock_info = shmget(ftok("msocket.h", SHM_MACRO2), sizeof(struct sock_info), 0777 | IPC_CREAT);
    shmctl(shm_sock_info, IPC_RMID, NULL);
    exit(0);
}


int m_recvfrom (int sockfd, char *buf, size_t len) {
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    pop.sem_num = vop.sem_num = sockfd;
    wait(sem_join);
    errno = 0;
    for (int i = 0; i < RWND; i++) {
        if (SM[sockfd].recv_status[i] == SM[sockfd].rwnd_markers[2]) {
            memcpy(buf, SM[sockfd].recv_buf[i], SM[sockfd].recv_len[i]);
            int len_backup = SM[sockfd].recv_len[i];
            if (len < len_backup) {
                signall(sem_join);
                pop.sem_num = vop.sem_num = 0;
                printf("Provided buffer size less than the message length\n");
                errno = ENOMSG;
                return -1;
            }
            #ifdef DEBUG
            printf("recvfrom at %d: ",i);
            for (int m = 0; m < SM[sockfd].recv_len[i]; m++) printf("%c",(SM[sockfd].recv_buf[i])[m]);
            printf("\n");
            fflush(stdout);
            #endif
            SM[sockfd].recv_status[i] = 0;
            SM[sockfd].recv_len[i] = 0;
            SM[sockfd].rwnd_markers[2] = (SM[sockfd].rwnd_markers[2]+1)%MAXSEQNO;
            if (SM[sockfd].rwnd_markers[2] == 0) SM[sockfd].rwnd_markers[2] = 1;
            // update window size
            #ifdef DEBUG
            printf("updating recv window at i: %d from (%d,%d) ",i, SM[i].rwnd_markers[0],SM[i].rwnd_markers[1]);
            fflush(stdout);
            #endif 
            if (SM[sockfd].rwnd.size < MAXWNDW) {
                SM[sockfd].rwnd.size++;
                SM[sockfd].rwnd_markers[1] = (SM[sockfd].rwnd_markers[1]+1)%RWND;
            }
            #ifdef DEBUG
            printf("to (%d,%d)\n",SM[i].rwnd_markers[0],SM[i].rwnd_markers[1]);
            fflush(stdout);
            #endif
            signall(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return len_backup;
        }
    }
    signall(sem_join);
    pop.sem_num = vop.sem_num = 0;
    errno = ENOMSG;
    return -1;
}

int m_sendto(int sockfd, char *buf, size_t len, char* dest_ip, int port) {  // added sockaddr to check for dest ip and port
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    if (len > MAXBLOCK) {
        printf("Maximum length allowed is 1024");
        return -1;
    }
    if (len < 0) {
        printf("Length specified cannot be negative");
        return -1;
    }
    // check if destination ip and port match 
    if (strcmp(SM[sockfd].dest_ip_addr, dest_ip) != 0 || SM[sockfd].dest_port != port) {
        errno = ENOTCONN;
        return -1;
    }
    
    pop.sem_num = vop.sem_num = sockfd;
    wait(sem_join);
    errno = 0;
    #ifdef DEBUG
    printf("sendto: send_mark (%d,%d) and SM[%d].swnd.seq_no[%d] : %d\n", SM[sockfd].swnd_markers[0], SM[sockfd].swnd_markers[1], sockfd, SM[sockfd].swnd_markers[0], SM[sockfd].swnd.seq_no[SM[sockfd].swnd_markers[0]]);
    fflush(stdout);
    printf("Send sequence no. before updates : %d\n", SM[sockfd].send_seq_no);
    fflush(stdout);
    #endif
    if (SM[sockfd].is_alloted == 1) {
        int iter = SM[sockfd].swnd_markers[0]; int flag = 0;
        while (iter < SWND && SM[sockfd].swnd.seq_no[iter] != -1 && flag < SWND) {
            flag++;
            iter = (iter+1)%SWND;
        }
        if (SM[sockfd].swnd.seq_no[iter] == -1) {
            SM[sockfd].send_len[iter] = len;
            memcpy(SM[sockfd].send_buf[iter], buf, SM[sockfd].send_len[iter]);
            SM[sockfd].swnd.seq_no[iter] = SM[sockfd].send_seq_no;
            SM[sockfd].send_seq_no = (SM[sockfd].send_seq_no+1)%MAXSEQNO;
            if (SM[sockfd].send_seq_no == 0) SM[sockfd].send_seq_no = 1;
            
            // SM[sockfd].send_status[iter] = 1;
            #ifdef DEBUG  
            printf("Copying at %d value on SM buf : ", iter, sockfd, buf);
            for (int m = 0; m < SM[sockfd].send_len[iter]; m++) printf("%c",(SM[sockfd].send_buf[iter])[m]);
            printf(" and send status : %d\n", SM[sockfd].swnd.seq_no[iter]);
            fflush(stdout);
            printf("Send sequence no. NOW : %d\n", SM[sockfd].send_seq_no);
            fflush(stdout);
            #endif
            signall(sem_join);
            pop.sem_num = vop.sem_num = 0;
            return len;
        }
    }
    signall(sem_join);
    pop.sem_num = vop.sem_num = 0;
    errno = ENOBUFS;
    return -1;
}

int m_socket(int domain, int type, int protocol){ 
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    if (type != SOCK_MTP) return -1;
    if (domain != AF_INET) return -1;
    // if (protocol != 0) return -1; // not checking for this as we have implmeneted only one protocol and this will be used in every case
    int sem_soc_create = semget(ftok("msocket.h", SEM_MACRO2), 3, 0777 | IPC_CREAT);
    int shm_sock_info = shmget(ftok("msocket.h", SHM_MACRO2), sizeof(struct sock_info), 0777 | IPC_CREAT);
    struct sock_info* sock_info = (struct sock_info*)shmat(shm_sock_info, NULL, 0);
    pop.sem_num = vop.sem_num = 2;
    wait(sem_soc_create);
    sock_info->sockfd = 0;
    sock_info->err = 0;
    pop.sem_num = vop.sem_num = 0;
    signall(sem_soc_create);
    pop.sem_num = vop.sem_num = 1;
    wait(sem_soc_create);
    errno = 0;
    if (sock_info->err != 0) {
        errno = sock_info->err;
        sock_info->err = 0;
        sock_info->sockfd=0;
        pop.sem_num = vop.sem_num = 2;
        signall(sem_soc_create);
        return -1;
    }
    int i;
    for (i=0; i<MAXSOCKETS; i++){
        pop.sem_num = vop.sem_num = i;
        wait(sem_join);
        if (SM[i].is_alloted == 0) {
            SM[i].is_alloted = 1;
            SM[i].process_id = getpid();
            SM[i].socket_id = sock_info->sockfd;
            signall(sem_join);
            sock_info->sockfd=0;
            pop.sem_num = vop.sem_num = 2;
            signall(sem_soc_create);
            return i;
        }
        signall(sem_join);
    }
    errno = ENOBUFS;
    sock_info->sockfd=0;
    pop.sem_num = vop.sem_num = 2;
    signall(sem_soc_create);
    return -1;
}

int m_bind(int sockfd, char* source_ip, int source_port, char* dest_ip, int dest_port){
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    int sem_soc_create = semget(ftok("msocket.h", SEM_MACRO2), 3, 0777 | IPC_CREAT);
    int shm_sock_info = shmget(ftok("msocket.h", SHM_MACRO2), sizeof(struct sock_info), 0777 | IPC_CREAT);
    struct sock_info* sock_info = (struct sock_info*)shmat(shm_sock_info, NULL, 0);
    pop.sem_num = vop.sem_num = 2;
    wait(sem_soc_create);
    sock_info->sockfd = SM[sockfd].socket_id;
    strcpy(sock_info->src_ip, source_ip);
    sock_info->src_port = source_port;
    sock_info->err = 0;
    pop.sem_num = vop.sem_num = 0;
    signall(sem_soc_create);
    pop.sem_num = vop.sem_num = 1;
    wait(sem_soc_create);
    if (sock_info->err != 0) {
        errno = sock_info->err;
        sock_info->err = 0;
        sock_info->sockfd=0;
        memset(sock_info->src_ip, 0, MAXIP);
        sock_info->src_port = 0;
        pop.sem_num = vop.sem_num = 2;
        signall(sem_soc_create);
        return -1;
    }
    pop.sem_num = vop.sem_num = sockfd;
    wait(sem_join);
    if (SM[sockfd].is_alloted == 1) {
        strcpy(SM[sockfd].src_ip_addr, source_ip);
        SM[sockfd].src_port = source_port;
        strcpy(SM[sockfd].dest_ip_addr, dest_ip);
        SM[sockfd].dest_port = dest_port;
        pop.sem_num = vop.sem_num = sockfd;
        signall(sem_join);
        sock_info->err = 0;
        sock_info->sockfd=0;
        memset(sock_info->src_ip, 0, MAXIP);
        sock_info->src_port = 0;  
        pop.sem_num = vop.sem_num = 2;
        signall(sem_soc_create);
        return 0;
    }
    sock_info->err = 0;
    sock_info->sockfd=0;
    memset(sock_info->src_ip, 0, MAXIP);
    sock_info->src_port = 0;
    errno = ENOBUFS;
    pop.sem_num = vop.sem_num = 2;
    signall(sem_soc_create);
    return -1;
}

int m_close(int fd){ 
    signal(SIGINT, sighandler);
    int sem_join = semget(ftok("msocket.h", SEM_MACRO), MAXSOCKETS, 0777 | IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;
    int sem_soc_create = semget(ftok("msocket.h", SEM_MACRO2), 3, 0777 | IPC_CREAT);
    int shm_sock_info = shmget(ftok("msocket.h", SHM_MACRO2), sizeof(struct sock_info), 0777 | IPC_CREAT);
    struct sock_info* sock_info = (struct sock_info*)shmat(shm_sock_info, NULL, 0);
    int shm_sockhand = shmget(ftok("msocket.h", SHM_MACRO), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    pop.sem_num = vop.sem_num = 2;
    wait(sem_soc_create);
    sock_info->sockfd = SM[fd].socket_id;
    sock_info->err = 0;
    sock_info->src_port = -1;
    pop.sem_num = vop.sem_num = 0;
    signall(sem_soc_create);
    pop.sem_num = vop.sem_num = 1;
    wait(sem_soc_create);
    if (sock_info->err != 0) {
        errno = sock_info->err;
        sock_info->err = 0;
        sock_info->sockfd=0;
        sock_info->src_port = 0;
        pop.sem_num = vop.sem_num = 2;
        signall(sem_soc_create);
        return -1;
    }
    pop.sem_num = vop.sem_num = fd;
    wait(sem_join);
    if (SM[fd].is_alloted == 1) {
        SM[fd].is_alloted = 0;
        memset(SM[fd].src_ip_addr, 0, MAXIP);
        memset(SM[fd].dest_ip_addr, 0, MAXIP);
        memset(SM[fd].send_buf, 0, 5*((int)MAXBLOCK));
        for (int i=0; i<SWND; i++) SM[fd].send_len[i] = 0;
        memset(SM[fd].recv_buf, 0, 10*((int)MAXBLOCK));
        for (int i=0; i<RWND; i++) SM[fd].recv_len[i] = 0;
        SM[fd].send_seq_no = 1;
        SM[fd].recv_seq_no = 1;
        int n = (int)MAXWNDW-1; int m = ((int)MAXSEQNO)/2;
        SM[fd].swnd_markers[0] = 0; SM[fd].swnd_markers[1] = n < m ? n : m;
        SM[fd].rwnd_markers[0] = 0; SM[fd].rwnd_markers[1] = n < m ? n : m;
        for (int j=0; j<(RWND > SWND ? RWND : SWND); j++){
            SM[fd].rwnd.seq_no[j] = -1;
            SM[fd].swnd.seq_no[j] = -1;
            if (j < SWND ) { /*SM[fd].send_status[j] = 0;*/ SM[fd].send_time[j].tv_sec = 0; SM[fd].send_time[j].tv_usec = 0; }
            if (j < RWND ) SM[fd].recv_status[j] = 0;
        }
        SM[fd].rwnd.size = MAXWNDW;
        SM[fd].swnd.size = MAXWNDW;
        SM[fd].rwnd_markers[2] = 1;
        SM[fd].flag_nospace = 0;
        signall(sem_join);
        sock_info->err = 0;
        sock_info->sockfd=0;
        sock_info->src_port = 0;
        pop.sem_num = vop.sem_num = 2;
        signall(sem_soc_create);
        return 0;
    }
    signall(sem_join);
    sock_info->err = 0;
    sock_info->sockfd=0;
    sock_info->src_port = 0;
    pop.sem_num = vop.sem_num = 2;
    signall(sem_soc_create);
    return -1;
}

int dropMessage(float pp) { 
    signal(SIGINT, sighandler);
    // generate a random number between 0 and 1
    srand(time(NULL));    
    float m_num = (float)rand()/(float)(RAND_MAX);
    if (m_num < pp) {
        return 1;
    }
    return 0;
}
