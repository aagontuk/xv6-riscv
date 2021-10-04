#include "kernel/types.h"
#include "user.h"
#include "libring.h"

int main(void) {
   int fd = rb_open("ring");
   printf("ring fd: %d\n", fd);
   printf("closing...\n");
   rb_close(fd);

   exit(0);
}
