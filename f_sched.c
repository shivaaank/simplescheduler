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
#include <stdbool.h>
#include<string.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>


const int SIZE = 100; //max size of ready queue

// int NCPU, TSLICE;


typedef struct {
    char *command;
    int pid;
    struct timeval starttime;
    long compltime;
    long waittime; // in microseconds
    struct timeval endtime;
    struct timeval timestop;
}Process;
typedef struct {
    Process items[SIZE];
    int front;
    int rear;
} readyQ;


// Function to initialize the queue
void initializereadyQ(readyQ* q)
{
    q->front = -1;
    q->rear = 0;
}

// Function to check if the readyQ is empty
bool isEmpty(readyQ* q) { return (q->front == q->rear - 1); }


// Function to add an element to the readyQ (EnreadyQ
// operation)
void enreadyQ(readyQ* q, Process *p)
{
    // if (isFull(q)) {
    //     printf("readyQ is full\n");
    //     return;
    // }
    q->items[q->rear] = *p;
    q->rear++;
}

// Function to remove an element from the readyQ (DereadyQ
// operation)
void dereadyQ(readyQ* q)
{
    if (isEmpty(q)) {
        printf("readyQ is empty\n");
        return;
    }
    q->front++;
}


// Function to print the current readyQ
void printreadyQ(readyQ* q)
{
    if (isEmpty(q)) {
        printf("readyQ is empty\n");
        return;
    }

    printf("Current readyQ: ");
    for (int i = q->front + 1; i < q->rear; i++) {
        printf("%s ", q->items[i].command);
    }
    printf("\n");
}
readyQ *q;
Process allp[100];
int allpi=0;

int shmid, shmid_proc;
char *buff;
Process * proc;
sem_t *lock;

void sigusr1(int sig){
    // printf("recvd\n");
    // int ok; sem_getvalue(lock, &ok);
    // printf("%d sem val(sched)\n", ok);
    // sem_wait(lock);
    char *cmd = buff;
    int pchild = fork();
    // printf("readdddd %s\n",cmd);
    // sleep(4);
    if(pchild==0){
        execl(cmd, cmd, NULL);
        printf("couldn't exec submitted program\n");
        exit(1);
    }
    Process p;
    p.command = buff;
    p.pid = pchild;
    p.waittime=0l;
    gettimeofday(&p.starttime, NULL);
    p.timestop=p.starttime;
    enreadyQ(q, &p);
    // printf("%d\n", getpid());
    // sem_post(lock);
    return;
}

void sigchld(int signum){
    // printf("finished\n");
    // if(waitpid(-1, NULL, WNOHANG)==-1){perror("w");return;}
    int st;
    int pid = waitpid(-1, &st, WNOHANG);
    // printf("pid = %d", pid);
    // dereadyQ(q);
    if(pid!=0){
        for(int i=q->front+1; i<q->rear; i++){
            if(q->items[i].pid==pid){
                gettimeofday(&q->items[i].endtime, NULL);
                allp[allpi++] = q->items[i];
                // printf("here");
                for(int j=i-1; j>=q->front; j--){
                    //swap i+j with i+j+1
                    // swap(q->items[i+j], q->items[i+j+1]);
                    // printf("swapped %d(i=%d) with %d(j=%d)\n", q->items[i],i, q->items[j],j);
                    Process *temp = &q->items[i];
                    q->items[i] = q->items[j];
                    q->items[j] = *temp;
                }
                // gettimeofday(&)
                dereadyQ(q);
            }
        }
        printreadyQ(q);
    }
    // printf("here");
}

void printprochistory(){
    printf("\n");
    for(int i=0; i<allpi; i++){
        printf("Job %d) %s\n", i+1, allp[i].command);
        // printf("start time = %ld\n", allp[i].starttime);
        // printf("end time = %ld\n", allp[i].endtime);
        printf("completion time: %ld millisecond\n", (allp[i].endtime.tv_sec-allp[i].starttime.tv_sec)  * 1000);
        printf("wait time: %ld microsecond\n", allp[i].waittime);
        printf("pid: %d\n", allp[i].pid);
    }
    printf("----------\n");
}
void sigint(int signum){
    // printf("recvd sigint");
    printprochistory();
    proc = q->items;
    while(!isEmpty(q)){};
    shmdt(buff);
    shmctl(shmid, IPC_RMID, NULL);
    if(q!=NULL)free(q);
    exit(0);
}
int min(int a, int b){
    if(a<b) return a;
    return b;
}
int main(int argc, char **argv){
    // printf("my pid %d\n", getpid());
    int *e;
    int TSLICE= strtol(argv[1], &e, 10);
    int NCPU=strtol(argv[2], &e, 10);

    TSLICE = 1000;
    NCPU = 1;
    
    signal(SIGUSR1, sigusr1);
    signal(SIGCHLD, sigchld);
    signal(SIGINT, sigint);
    q = (readyQ*)malloc(sizeof(readyQ));
    initializereadyQ(q);
    key_t key = ftok("shmfile", 65);
    // key_t key2 = ftok("shmfilee", 66);
    // key_t shmkey = ftok("/dev/null", 5);
    // shmget returns an identifier in shmid
    shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    // shmid_proc = shmget(key2, sizeof(Process)*100, 0666 | IPC_CREAT);
    // int shmidsem = shmget (shmkey, sizeof (int), IPC_EXCL);
    // if (shmid < 0){                           /* shared memory error check */
        // perror ("shmget\n");
        // exit (1);
    // }

    // shmat to attach to shared memory
    buff = (char*)shmat(shmid, (void*)0, 0);
    // proc = (Process*)shmat(shmid_proc, (void*)0, 0);
    // int* p = (int*)shmat (shmidsem, NULL, 0);
    // lock = sem_open("pSem", O_EXCL);
    // int ok; sem_getvalue(lock, &ok);
    // printf("%d sem val\n", ok);
    // int st=fork();
    // if(st==0){
    //     execl("./fib40", "./fib40", NULL);
    // }
    // Process np;
    // np.command = "./fib40";
    // np.pid = st;
    // // sleep(1);
    // np.waittime=0;
    // gettimeofday(&np.starttime, NULL);
    // np.timestop = np.starttime;
    // enreadyQ(q,&np);
    // printreadyQ(q);
    while(1){
        if(!isEmpty(q)){
            for(int i=q->front+1; i<min(q->front+1+NCPU, q->rear); i++){
                Process *proc = &q->items[i];
                // printf("\nrunning process %d\n", proc->pid);
                struct timeval curr; gettimeofday(&curr, NULL);
                long duration = (curr.tv_sec - proc->timestop.tv_sec) * 1000000 + curr.tv_usec - proc->timestop.tv_usec; 
                // printf("cmd: %s\n addr = %x\n", proc->command, &proc);
                // printf("waittime before = %ld\n", proc->waittime);
                // printf("duration = %ld\n", duration);
                proc->waittime+=duration;
                // printf("waittime after = %ld\n", proc->waittime);
                kill(proc->pid, SIGCONT);
            }
            sleep(TSLICE/1000);
            int size = q->rear-q->front-1;
            if(!isEmpty(q)){
                readyQ *cpy = (readyQ *)malloc(sizeof(readyQ));
                memcpy(cpy, q, sizeof(readyQ));
                // printf("q front = %d\nq rear = %d\n", cpy->front, cpy->rear);
                for(int i=cpy->front+1; i<min(cpy->front+1+NCPU, cpy->rear); i++){
                    Process proc = cpy->items[i];
                    // printf("\nstopped process %d\n", proc.pid);
                    kill(proc.pid, SIGSTOP);
                    gettimeofday(&proc.timestop, NULL);
                    // printf("time stop = %ld\n", proc.timestop);
                    enreadyQ(q, &proc);
                    dereadyQ(q);
                }
                free(cpy);
            }
        }
    }
    // printf("data read: %s\n", str);

    // detach from shared memory

    // shmdt(buff);
    // free(q);

    // destroy the shared memory
    // shmctl(shmid, IPC_RMID, NULL);


}