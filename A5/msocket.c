#include "msocket.h"

#define wait(s) semop(s, &pop, 1) 
#define signal(s) semop(s, &vop, 1) 

static int sem_join = semget(IPC_PRIVATE, MAXSOCKETS, 0777|IPC_CREAT);
for (int i=0; i<MAXSOCKETS; i++) semctl(sem_join, i, SETVAL, 1);

struct sembuf pop, vop ;
pop.sem_num = vop.sem_num = 0;
pop.sem_flg = vop.sem_flg = 0;
pop.sem_op = -1; vop.sem_op = 1;

// ???? is there a limit ot the socket calls or the bind calls
// ???? use semaphores to protect the shared memory
// ???? how to check for dest ip and port if we are not sending that

void* R(){
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    while (1){
        fd_set fds;
        FD_ZERO(&fds);
        // add all the fds in socket_id of SM
        for(int i=0; i<MAXSOCKETS; i++) 
        {
            pop.sem_num = vop.sem_num = i;
            wait(sem_join);
            if (SM[i].is_alloted != 0) FD_SET(SM[i].socket_id, &fds);
            signal(sem_join);
            pop.sem_num = vop.sem_num = 0;
        }
        select(FD_SETSIZE, &fds, NULL, NULL, NULL); // ???? set the timer
        
    }
    pthread_exit(NULL);
}

void* S(){
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    pthread_exit(NULL);
}

void* G(){
    pthread_exit(NULL);
}

int m_socket(int domain, int type, int protocol){
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
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    
    int flag = 0;
    for (int i=0; i<MAXSOCKETS; i++){
        pop.sem_num = vop.sem_num = i;
        wait(sem_join);
        if (SM[i].is_alloted == 1 && SM[i].socket_id == sockfd){
            flag = 1;
            strcpy(SM[i].src_ip_addr, source_ip);
            SM[i].src_port = source_port;
            strcpy(SM[i].dest_ip_addr, dest_ip);
            SM[i].dest_port = dest_port;
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons((unsigned short)source_port);
            addr.sin_addr.s_addr = inet_addr(source_ip);
            if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
            {
                signal(sem_join);
                pop.sem_num = vop.sem_num = 0;
                return -1;
            }
        }
        signal(sem_join);
        pop.sem_num = vop.sem_num = 0;
    }
    if (flag == 0) return -1;
}

int m_close(int fd){
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
        }
        signal(sem_join);
        pop.sem_num = vop.sem_num = 0;
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