#include "dummy_main.h"

int fib(int x){
    if(x==1 | x==0) return 1;
    return fib(x-1)+fib(x-2);
}
// void handle_sigcont(int signo) {
//     // Do nothing, just interrupt sigsuspend to resume execution
//     // printf("received sigcont");
// }
// void sigint(int signo){
//     return;
// }
int main(int argc, char **argv){
    // signal(SIGCONT, handle_sigcont);
    // pause();
    // printf("started now.\n");
    // signal(SIGCONT, handle_sigcont);
    // signal(SIGINT, sigint);
    // printf("%d\n", getpid());
    // pause();
    int ans = fib(43);
    printf("\nfib fin, %d\n", ans);
    return 0;
}