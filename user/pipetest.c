/* Assignment: 1 */

#include "kernel/types.h"
#include "user/user.h"

#define BUFSIZE 128
#define DATASIZE 10485760

int main(void) {
    int p[2];
    int pid;
    int nsend = DATASIZE/(BUFSIZE*4);
    int nbytes, tbytes = 0;
    int writebuffer[BUFSIZE];
    int readbuffer[BUFSIZE];

    printf("ticks: %d\n", uptime());
    if (pipe(p)) {
        fprintf(2, "pipe() failed!\n");
        exit(-1);
    };
    
    pid = fork();

    if (pid == -1) {
        fprintf(2, "fork() failed!\n"); 
        exit(-2);
    }
    else if (pid == 0) {
        close(p[0]);
        for (int n = 0; n < nsend; n++) {
            for (int i = 0; i < BUFSIZE; i++) {
                writebuffer[i] = i;
            }
            write(p[1], (void *)writebuffer, sizeof(int)*BUFSIZE);
        }
        close(p[1]);
    }
    else {
        close(p[1]);
        for (int n = 0; n < nsend; n++) {
            nbytes = read(p[0], (void *)readbuffer, sizeof(int)*BUFSIZE);
            tbytes += nbytes;
        }
        close(p[0]);
        wait((int *)0);
        fprintf(1, "Read: %d\n", tbytes);
        printf("ticks: %d\n", uptime());
    }
    
    exit(0);
}
