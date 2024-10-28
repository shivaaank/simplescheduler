#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct shm_t {
    int A[100];
    int head;
    int tail;
} readyQ;


int main(){
    // int shmid;
    // key_t key;
    // char *shm;
    // readyQ *q;
    // key = 5678;
    // if ((shmid = shmget(key, sizeof(readyQ), IPC_CREAT | 0666)) < 0) {
    //     perror("shmget");
    //     exit(1);
    // }
    // if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    //     perror("shmat");
    //     exit(1);
    // }
    // q = (readyQ *)shm;
    // q->head = 0;
    // q->tail = 0;
    // //enqueue
    // for(int i=0; i<5; i++){
    //     q->A[q->tail] = i;
    //     q->tail++;
    // }
    int st = fork();
    if(st==0){
        execl("./fib40", "./fib40", NULL);
        perror("yo");
    }else{
        int stat;
        printf("%d", st);
        kill(st, SIGCONT);
        waitpid(st,&stat,0);
    }
    // if(st==0){
    //     sleep(1);
    // }
    // else{
    //     int status;
    //     int k = waitpid(st, &status, WNOHANG|WUNTRACED);
    //     printf("%d, %d\n",k, status);
    //     sleep(3);
    //     kill(st, SIGSTOP);
    //     k = waitpid(st, &status, WNOHANG|WUNTRACED);
    //     printf("%d, %d\n",k, WIFSTOPPED(status));
    // }
}