#include "dummy_main.h"
int main(int argc, char **argv){

    printf("I am process 1\n\n");

        //Loop for print check
    for (int a = 0; a < 5; a++) {
        printf("Process %d is in running state... %d\n\n", getpid(), a + 1);
        sleep(1);
    }
        printf("Process 1 is finished\n\n");
    return 0;
}