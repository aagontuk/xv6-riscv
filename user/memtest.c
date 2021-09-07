#include "kernel/types.h"
#include "user/user.h"

void benchmark(void);

int main(void) {
    int from[4] = {1, 2, 3, 4};
    int to[4] = {7, 8, 9, 10};

    memcpy64(to, from, 16);

    printf("%d\t%d\t%d\t%d\n", to[0], to[1], to[2], to[3]);

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
        memcpy64(to, from, 4096);
        /*memmove(to, from, 4096);*/
    }
    end = uptime();

    printf("took: %d ticks\n", end - start);
}
