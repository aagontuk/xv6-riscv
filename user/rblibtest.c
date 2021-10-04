#include "kernel/types.h"
#include "user.h"
#include "libring.h"

int main(void) {
    int pid; 
    int pfd;
    char **buf = 0;
    char *str;
    int avail;

    printf("parent opening ring...\n");
    pfd = rb_open("ring");
    printf("fd: %d\n", pfd);
    
    rb_write_start(pfd, buf, &avail);
    printf("available %d from %p\n", avail, *buf);
    
    str = *buf;
    strcpy(str, "hello");
    rb_write_finish(pfd, 5);

    pid = fork();

    if(pid == 0) {
        int cfd;
        char **cbuf = 0;
        char *cstr;
        int ravail;
        
        printf("child opening ring..\n"); 
        cfd = rb_open("ring");
        printf("fd: %d\n", cfd);
        
        rb_read_start(cfd, cbuf, &ravail); 
        printf("available %d from %p\n", ravail, *cbuf);

        cstr = *cbuf;
        printf("Child read: %s\n", cstr);
        rb_read_finish(cfd, 5);
        
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
