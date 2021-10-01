#include "kernel/types.h"
#include "user/user.h"

int main(void) {
    char *ptr = 0;
    ringbuf("hello", 0, (void *)ptr);
    ringbuf("hello", 0, (void *)ptr);
    ringbuf("hello", 0, (void *)ptr);
    printf("Time to die!\n");
    exit(0);
}
