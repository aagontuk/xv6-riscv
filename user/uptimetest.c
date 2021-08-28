#include "kernel/types.h"
#include "user/user.h"

int main (void) {
    printf("ticks: %d\n", uptime());
    exit(0);
}
