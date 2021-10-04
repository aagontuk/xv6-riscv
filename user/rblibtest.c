#include "kernel/types.h"
#include "user.h"
#include "libring.h"

int main(void) {
    int pid; 
    int pfd;
    char **buf = 0;
    int avail;

    printf("parent opening ring...\n");
    pfd = rb_open("ring");
    printf("fd: %d\n", pfd);
    
    rb_write_start(pfd, buf, &avail);
    printf("available %d from %p\n", avail, *buf);

    pid = fork();

    if(pid == 0) {
        int cfd;
        printf("child opening ring..\n"); 
        cfd = rb_open("ring");
        printf("fd: %d\n", cfd);
        printf("child closing ring..\n"); 
        rb_close(cfd);
    }
    else {
        wait((int *)0);
        printf("parent closing ring..\n"); 
        rb_close(pfd);
    }

    exit(0);
}
