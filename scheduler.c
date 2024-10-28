#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>

const int SIZE = 100; //max size of ready queue

int NCPU, TSLICE;

typedef struct shm_t {
    sem_t lock;
    int A[SIZE];
    int head;
    int tail;
} readyQ;

int main(int argc, char* argv[]){
    
    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);
    // printf("here in child");

    int shmid;
    key_t key;
    char *shm;
    readyQ *q;
    /*
     * We need to get the segment named
     * "5678", created by the server.
     */
    key = 5678;

    /*
     * Locate the segment.
     */
    if ((shmid = shmget(key, sizeof(readyQ), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    /*
     * Now read what the server put in the memory.
     */
    q = (readyQ *)shm;
    // printf("elements in q are: ");
    // for(int i=q->head; i<q->tail; i++){
    //     printf("%d : %d\n", i, q->A[i]);
    // }
    while(1){
        // printf("queue head = %d", q->head);
        sem_wait(&q->lock);
        if(q->head!=q->tail-1){
            // printf("aha\n");
            int lim = q->tail;
            if(q->tail-q->head > NCPU) lim = q->head+NCPU;
            for(int i=q->head+1; i<=lim; i++){
                int proc = q->A[i];
                printf("\nrunning process %d\n", proc);
                kill(proc, SIGCONT);
            }
            sem_post(&q->lock);
            sleep(TSLICE/1000);
            // printf("yooo awake\n");
            sem_wait(&q->lock);
            for(int i=q->head+1; i<=lim; i++){
                int proc = q->A[i];
                printf("\nstopped process %d\n", proc);
                kill(proc, SIGSTOP);
                int status;
                int pid = waitpid(proc, &status, WNOHANG);
                if (pid == -1) {
                    if (errno == ECHILD) {
                        printf("Process %d has already exited\n", proc);
                    } else {
                        perror("waitpid error");
                    }
                 }
                perror("yo\n");
                printf("pid = %d", pid);
                if(WIFSTOPPED(status))
                {
                    //process not terminated yet so add to end of queue;
                    int pid = q->A[q->head+1];
                    printf("adding to queue again %d\n", pid);
                    q->head++; //dequeue
                    //enqueue
                    q->A[q->tail] = pid;
                    q->tail++;
                }
                else
                {
                    //process has terminated so just dequeue;
                    printf("dequeued %d\n", pid);
                    q->head++;
                }
            }
            sem_post(&q->lock);
        }
    }
    shmdt(q);
}