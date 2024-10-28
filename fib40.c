#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int fib(int x){
    if(x==1 | x==0) return 1;
    return fib(x-1)+fib(x-2);
}
void handle_sigcont(){
    
}
int main(){
    sigset_t newmask, oldmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGCONT);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);
    struct sigaction sa;
    sa.sa_handler = handle_sigcont;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCONT, &sa, NULL);
    int ans = fib(40);
    printf("fib fin, %d\n", ans);
    return 0;
}