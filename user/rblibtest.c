#include "kernel/types.h"
#include "user.h"
#include "libring.h"

int main(void) {
    int pid; 
    int pfd;

    printf("parent opening ring...\n");
    pfd = rb_open("ring");
    printf("fd: %d\n", pfd);
    printf("writing to book...\n");
    bookw(pfd);    

    pid = fork();

    if(pid == 0) {
        int cfd;
        printf("child opening ring..\n"); 
        cfd = rb_open("ring");
        printf("fd: %d\n", cfd);
        printf("reading from book...\n");
        bookr(pfd);    
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
