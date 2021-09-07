#include "kernel/types.h"
#include "user/user.h"

void benchmark(void);

int main(void) {
    benchmark();

    exit(0);
}

void benchmark(void) {
    char *from = (char *)malloc(4096);
    char *to = (char *)malloc(4096);
    int i;
    int start, end;

    for (i = 0; i < 4096; i++) {
        from[i] = i;
    }
    
    start = uptime();
    for (i = 0; i < 1000000; i++) {
        memmove(to, from, 4096);
    }
    end = uptime();
    printf("memmove() took: %d ticks\n", end - start);

    start = uptime();
    for (i = 0; i < 1000000; i++) {
        memcpy64(to, from, 4096);
    }
    end = uptime();
    printf("memcpy64() took: %d ticks\n", end - start);
    
    start = uptime();
    for (i = 0; i < 1000000; i++) {
        memcpy64_aln(to, from, 4096);
    }
    end = uptime();
    printf("memcpy64_aln() took: %d ticks\n", end - start);
    
    start = uptime();
    for (i = 0; i < 1000000; i++) {
        memcpy64_aln_bit(to, from, 4096);
    }
    end = uptime();
    printf("memcpy64_aln_bit() took: %d ticks\n", end - start);
}
