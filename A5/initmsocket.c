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
        SM[i].send_seq_no = 1;
        SM[i].recv_seq_no = 1;
        SM[i].swnd_markers[0] = 0; SM[i].swnd_markers[1] = (MAXWNDW-1) < MAXSEQNO/2 ? (SWND-1) : MAXSEQNO/2;
        SM[i].rwnd_markers[0] = 0; SM[i].rwnd_markers[1] = (MAXWNDW-1) < MAXSEQNO/2 ? (RWND-1) : MAXSEQNO/2;
        for (int j=0; j<(RWND > SWND ? RWND : SWND); j++){
            SM[i].rwnd.seq_no[j] = -1;
            SM[i].swnd.seq_no[j] = -1;
            if (j <= SWND )SM[i].send_status[j] = 0;
            if (j <= RWND )SM[i].recv_status[j] = 0;
        }
        SM[i].rwnd.size = MAXWNDW;
        SM[i].swnd.size = MAXWNDW;
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
