#include <sys/time.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

typedef struct {
    char *command;
    int pid;
    struct timeval time;
    long duration; // in microseconds
}Command ;

typedef struct {
    char *command;
    int pid;
    struct timeval starttime;
    long compltime;
    long waittime; // in microseconds
    struct timeval endtime;
    struct timeval timestop;
}Process;

#define n 1024 //max length of a command

Command history[100]; //array to store commands
int curcommands = 0; //number of commands executed

void storecmd(char *cmd, int pid, struct timeval start, long duration) {
    //Funtion to store command.
    if (curcommands < 100) {
        history[curcommands].command = strdup(cmd);
        history[curcommands].pid = pid;
        history[curcommands].time = start;
        history[curcommands].duration = duration;
        curcommands++;
    }
}

void printcmdhistory(bool deets) {
    //Funtion to print history
    for (int i = 0; i < curcommands; i++) {
        struct timeval start = history[i].time;
        long seconds = start.tv_sec + 5*3600 + 30*60; //adjust for IST because linux follows GMT by default.
        long useconds = start.tv_usec; 
        seconds = seconds%86400; //tv_sec gives number of seconds since start of the year so mod by seconds in a day.
        long hours=seconds/3600;
        seconds=seconds-hours*3600;
        long minutes=seconds/60;
        seconds=seconds-minutes*60;
         
        printf("Command: %s\n", history[i].command);
        if(deets){ // when exiting
            printf("Program ID: %d\n", history[i].pid);
            printf("Time: %ld Hours and %ld minutes and %ld seconds and %ld microseconds\n", hours,minutes,seconds, useconds);
            printf("Duration: %ld microseconds\n", history[i].duration);
            printf("\n");
        }
    }
}


void handle_sigchld(int sig){
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("process %d finished.\n", pid);
    return;
}

int pids;

void handle_ctrlc(int sig) {
    // Signal handler for Ctrl+C (SIGINT)
    int stat;
    kill(pids, SIGINT);
    printf("waiting for jobs to finish.\n");
    printf("--------------\n");
    wait(&stat);
    printf("exit stat of sched = %d", WIFEXITED(stat));
    printcmdhistory(true); // Show command history with details
    // printprochistory();
    exit(0);
}

int main() {
    char TSLICE[100], NCPU[100];
    printf("enter time slice(in ms): ");
    fgets(TSLICE, 100, stdin);
    printf("enter NCPU: ");
    fgets(NCPU, 100, stdin);
    char fullcmd[n]; //array to store current command.
    signal(SIGINT, handle_ctrlc);
    key_t key = ftok("shmfile", 65);
    // key_t key2 = ftok("shmilee", 66);
    // key_t shmkey = ftok ("/dev/null", 5);

    // shmget returns an identifier in shmid
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    // int shmid_proc = shmget(key2, sizeof(Process)*100, 0666 | IPC_CREAT);
    // if (shmid_proc<0) perror("shmget\n");
    // int shmidsem = shmget (shmkey, sizeof (int), 0644 | IPC_CREAT);
    // if (shmid < 0){                           /* shared memory error check */
    //     perror ("shmget\n");
    //     exit (1);
    // }

    // shmat to attach to shared memory
    char* buff = (char*)shmat(shmid, (void*)0, 0);
    // p = (Process*)shmat(shmid_proc, (void*)0, 0);
    // int* p = (int*)shmat (shmidsem, NULL, 0);
    // sem_t *lock = sem_open("pSem", O_CREAT, 0644, 1);
    // int ok; sem_getvalue(lock, &ok);
    // printf("%d sem val(shell)\n", ok);
    // signal(SIGCHLD, handle_sigchld);
    pids = fork();
    if(pids==0){
        execl("./sched", "./sched", TSLICE, NCPU, NULL);
        perror("error forking sched\n");
        exit(1);
    }
    sleep(1);

    // ftok to generate unique key
    // printf("Write Data : ");
    // fgets(str, 1000, stdin);

    // printf("data written: %s\n", str);

    do {
        printf("AS'S> ");
        fgets(fullcmd, n, stdin);
        // printf("shell recvd: %s", fullcmd);
        // Remove newline character from the command
        fullcmd[strcspn(fullcmd, "\n")] = '\0';
        if(strcmp(fullcmd, "\n")==0 || strcmp(fullcmd, "")==0)continue;
        
        char *copy = strdup(fullcmd); //copy command to store because strtok changes original.
        struct timeval start, end;
        gettimeofday(&start, NULL);
        long duration;
        int pid;

        if (strcmp(fullcmd, "exit") == 0) {
            // Show the command history with details
            // printcmdhistory(true);
            handle_ctrlc(1);
            break;
        }
        else if (strcmp(fullcmd, "history") == 0) {
            // Show history
            pid=getpid();
            printcmdhistory(false);
        }
        
        else if(strncmp(fullcmd, "submit", strlen("submit"))==0){
            //send signal to sched to queue proc
            int commands = 0;
            char* cmd[100];
            for (char *p = strtok(fullcmd," "); p != NULL; p = strtok(NULL, " ")) {
                if(strcmp(p, "submit")==0)continue;
                cmd[commands] = p;
                // int ok; sem_getvalue(lock, &ok);
                // printf("%d sem val(shell)\n", ok);
                // sem_wait(lock);
                printf("writing %s\n", p);
                snprintf(buff, sizeof(buff), "%s", p);
                kill(pids, SIGUSR1);
                // sleep(3);
                // sem_post(lock);
                commands++;
            }
            // printf("written\n");
        }
        else{
            int commands=0; //total number of commands in the full command, will be >1 in case of pipe(s).

            char* cmds[100]; //to store each command in the full command.
            for(char *c = strtok(fullcmd, "|"); c!=NULL; c=strtok(NULL, "|")){
                cmds[commands]=c;
                commands++;
            }

            int k=0;
            char *andcopy = strdup(cmds[commands-1]); //duplicate last commmand
            char *andcheck[100]; //to store the last command and its arguments
            for(char *p = strtok(andcopy," "); p != NULL; p = strtok(NULL, " ")) {
                andcheck[k] = p;
                k++;
            }
            bool check=false;
            if(strcmp(andcheck[k-1], "&") == 0){ //if last command has "&", the whole command needs to run in background
                check=true;
            }

            int prev=-1;
            for(int i=0;i<commands;i++){ // going through each commands seperately 
                int fd[2];
                pipe(fd);
                char *c=cmds[i];
                char *args[100];

                // Parse the command and its arguments
                int j = 0;
                for (char *p = strtok(c," "); p != NULL; p = strtok(NULL, " ")) { // creating array with words seperately of the commands
                    args[j++] = p;           
                }
                if(i==commands-1 && strcmp(args[j-1], "&") == 0){ //if current command has "&"
                    args[j-1]=NULL;
                }

                args[j] = NULL;  // NULL-terminate the array

                // Create a child process to execute the command
                pid = fork();

                int status;
                if (pid == 0) {  // child process 
                    if(prev!=-1){   //if not first take input from pipe
                        dup2(prev,0);
                        close(prev);
                    }
                    if(i!=commands-1){  // if not last write to pipe
                        dup2(fd[1],1);
                        close(fd[1]);
                    }
                    // Child process: execute the command
                    if (execvp(args[0], args) == -1) {
                        perror("Command issue\n");
                    }
                    exit(EXIT_FAILURE);
                } 
                else if (pid > 0) { // parent process
                    if (prev != -1) {   // close prev if not first
                    
                        close(prev);
                    }
                    close(fd[1]);  
                    prev = fd[0];
                if(!check){
                    wait(&status);}
                } else {
                    
                    perror("Couldn't fork");
                }
            }
        }
        gettimeofday(&end, NULL);
        duration = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);  // calculating durating in secs and microsecs        
        storecmd(copy, pid, start, duration);
    
    }while(1);

    // Cleanup
    shmdt(buff);
    shmctl(shmid, IPC_RMID, NULL);
    // shmdt (p);
    // shmctl (shmid, IPC_RMID, 0);
    return 0;
}