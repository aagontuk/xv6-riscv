#include "kernel/types.h"
#include "user/user.h"

void str_test(uint64 addr) {
    char *s = (char *)addr;
    char *t = s + (4096*16);
    char *u = s + 4096;
    char *v = s + (4096*17);

    printf("\n\nstring test:\n\n");

    strcpy(s, "xv6");
    strcpy(u, "riscv");

    printf("first pair: (va+0, va+(4096*16))\n");
    printf("s:\taddr:%p\tval:%s\n", s, s);
    printf("t:\taddr:%p\tval:%s\n", t, t);
    
    
    printf("\nsecond pair: (va+4096, va+(4096*17))\n");
    printf("u:\taddr:%p\tval:%s\n", u, u);
    printf("v:\taddr:%p\tval:%s\n", v, v);
}

void int_test(uint64 addr) {
    int *x = (int *)addr;
    int *y = (int *)(addr + (4096*16));

    int *w = (int *)(addr + 4096);
    int *v = (int *)(addr + (4096*17));

    printf("\n\ninteger test:\n\n");
    
    *x = 10;
    *w = 100;
    
    printf("first pair: (va+0, va+(4096*16))\n");
    printf("addr: %p\tval:%d\n",x, *x);
    printf("addr: %p\tval:%d\n", y, *y);
    
    printf("\nsecond pair: (va+4096, va+(4096*17))\n");
    printf("addr: %p\tval:%d\n",w, *w);
    printf("addr: %p\tval:%d\n", v, *v);
}

void rbwrite(void *addr) {
    strcpy((char *)addr, "UTAH");
}

void rbread(void *addr) {
    printf("%s\n", (char *)addr);
}

int main(void) {
    void *addr;
    int pid;

    printf("Parent writing...\n");
    ringbuf("ring", 0, (uint64)&addr);
    rbwrite(addr);

    pid = fork();
    if(!pid) {
        printf("Child reading...\n");
    
        ringbuf("ring", 0, (uint64)&addr);
        rbread(addr);
        ringbuf("ring", 1, (uint64)&addr);
    }
    else {
        wait((int *)0);
        ringbuf("ring", 1, (uint64)&addr);
    }
    
    exit(0);
}
