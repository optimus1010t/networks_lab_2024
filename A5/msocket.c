#include "msocket.h"

void* R(){
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    printf("rrrrrr\n");
    pthread_exit(NULL);
}

void* S(){
    int shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    printf("ssssss\n");
    pthread_exit(NULL);
}

void* G(){
    pthread_exit(NULL);
}

int dropMessage(float pp) {
    // generate a random number between 0 and 1
    float m_num = (float)rand()/(float)(RAND_MAX);
    if (m_num < pp) {
        return 1;
    }
    return 0;
}