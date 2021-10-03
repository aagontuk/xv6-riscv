#include "kernel/types.h"
#include "user/user.h"

int main(void) {
    char *addr = (char *)50; 
    /*int index = 4096;*/
    /*char *str;*/
    ringbuf("hello", 0, addr);
    printf("%d\n", addr);
    exit(0);
}
