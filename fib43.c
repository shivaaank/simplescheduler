#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
int fib(int x){
    if(x==1 | x==0) return 1;
    return fib(x-1)+fib(x-2);
}
void handle_sigcont(int signo) {
    // Do nothing, just interrupt sigsuspend to resume execution
    printf("received sigcont");
}
int main(){
    signal(SIGCONT, handle_sigcont);
    pause();
    printf("started now.\n");
    int ans = fib(43);
    printf("fib fin, %d\n", ans);
    return 0;
}