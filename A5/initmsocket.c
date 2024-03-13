#include "msocket.h"

int main() {
    int shm_sockhand;
    shm_sockhand = shmget(ftok("msocket.h", 6), MAXSOCKETS*sizeof(struct m_socket_handler), 0777 | IPC_CREAT);
    struct m_socket_handler* SM = (struct m_socket_handler*)shmat(shm_sockhand, NULL, 0);
    if(shm_sockhand == -1){
        perror("shmget");
        exit(1);
    }
    for(int i=0; i<MAXSOCKETS; i++){
        SM[i].is_alloted = 0;
        memset(SM[i].src_ip_addr, 0, MAXIP);
        memset(SM[i].dest_ip_addr, 0, MAXIP);
        memset(SM[i].send_buf, 0, 5*MAXBLOCK);
        memset(SM[i].recv_buf, 0, 10*MAXBLOCK);
        SM[i].send_seq_no = 0;
        SM[i].recv_seq_no = 0;
        for (int j=0; j<2; j++){
            SM[i].swnd_markers[j] = 0;
            SM[i].rwnd_markers[j] = 0;
        }
        for (int j=0; j<SWND; j++){
            SM[i].send_status[j] = -1;
        }
        for (int j=0; j<RWND; j++){
            SM[i].recv_status[j] = -1;
        }
    }
    pthread_t Rid, Sid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&Rid, &attr, R, NULL);
    pthread_create(&Sid, &attr, S, NULL);

    pthread_join(Rid, NULL);
    pthread_join(Sid, NULL);

    shmctl(shm_sockhand, IPC_RMID, NULL);
    return 0;
}
