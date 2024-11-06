#include "dummy_main.h"

int fib(int x){
    if(x==1 | x==0) return 1;
    // printf("in %d", getpid());
    return fib(x-1)+fib(x-2);
}

int main(int argc, char **argv){
    int ans = fib(40);
    printf("\nfib fin, %d\n", ans);
    return 0;
}