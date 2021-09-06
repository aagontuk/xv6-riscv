/* 
 * Assignment: 1 
 * Md Ashfaqur Rahaman
 * 
 */

#include "kernel/types.h"
#include "user/user.h"

#define BUFSIZE 256     // 512 byte buffer
#define DATASIZE 10485760 // 10 MB

// from FreeBSD.
int
do_rand(unsigned long *ctx)
{
/*
 * Compute x = (7^5 * x) mod (2^31 - 1)
 * without overflowing 31 bits:
 *      (2^31 - 1) = 127773 * (7^5) + 2836
 * From "Random number generators: good ones are hard to find",
 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
 * October 1988, p. 1195.
 */
    long hi, lo, x;

    /* Transform to [1, 0x7ffffffe] range. */
    x = (*ctx % 0x7ffffffe) + 1;
    hi = x / 127773;
    lo = x % 127773;
    x = 16807 * lo - 2836 * hi;
    if (x < 0)
        x += 0x7fffffff;
    /* Transform to [0, 0x7ffffffd] range. */
    x--;
    *ctx = x;
    return (x);
}

unsigned long rand_next = 1;

int
rand(void)
{
    return (do_rand(&rand_next));
}

int main(void) {
    int p[2];
    int pid;
    int start, end;
    int nsend = DATASIZE/(BUFSIZE*4);
    int rbytes, tbytes = 0;
    int wbytes;
    int writebuffer[BUFSIZE];
    int readbuffer[BUFSIZE];

    if (pipe(p)) {
        fprintf(2, "pipe() failed!\n");
        exit(-1);
    };
    
    pid = fork();

    if (pid == -1) {
        fprintf(2, "fork() failed!\n"); 
        exit(-1);
    }
    else if (pid == 0) {
        printf("clidpid: %d\n", getpid());
        close(p[0]);
        
        for (int n = 0; n < nsend; n++) {
            for (int i = 0; i < BUFSIZE; i++) {
                writebuffer[i] = rand();
            }
            
            wbytes = write(p[1], (void *)writebuffer, sizeof(int)*BUFSIZE);
            if (wbytes == -1) {
                fprintf(2, "Error writing to pipe!\n"); 
                exit(-1);
            }
            else if (wbytes < sizeof(int)*BUFSIZE) {
                fprintf(2, "Error writing to pipe!\n"); 
                exit(-1);
            }
        }
        
        close(p[1]);
    }
    else {
        close(p[1]);
	    start = uptime();
        
        for (int n = 0; n < nsend; n++) {
            rbytes = read(p[0], (void *)readbuffer, sizeof(int)*BUFSIZE);
            
            if (rbytes == -1) {
                fprintf(2, "Error reading from pipe!\n");
                exit(-1);
            }
            
            tbytes += rbytes;

            /* Check integers */
            for (int i = 0; i < BUFSIZE; i++) {
                if (readbuffer[i] != rand()) {
                    fprintf(2, "Data miss match!\n"); 
                    exit(-3);
                } 
            }
        }
	    
        end = uptime();
        close(p[0]);
        wait((int *)0);
        
        fprintf(1, "Received: %d bytes in ", tbytes);
	    fprintf(1, "%d ticks.\n", end - start);
    }
    
    exit(0);
}
