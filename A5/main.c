#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>


int main (){
    struct timeval t1;
    struct timeval t2;
    gettimeofday(&t1, NULL);
    printf("Seconds: %ld\n", t1.tv_sec);
    printf("Microseconds: %ld\n", t1.tv_usec);
    sleep(5);
    gettimeofday(&t2, NULL);
    printf("Seconds: %ld\n", t2.tv_sec);
    printf("Microseconds: %ld\n", t2.tv_usec);
    // find the duration between t1 and t2 in seconds
    long int duration = ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec)/1000000;

}