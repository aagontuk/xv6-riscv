#include "kernel/types.h"
#include "user.h"
#include "libring.h"

#define DATASIZE (4096*1024)
#define NINT (DATASIZE/4)
#define WCHUNK (4096)

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
    int pid;
    
    pid = fork();

    if (pid == 0) {
        int cfd; 
        int cavailable = 0;
        int nread = 0;
        void *rstart = 0;
        int *cptr;
        int missmatch = 0;
        int match = 0;

        // open reader
        cfd = rb_open("ring");
        
        // poll until all the data read
        while (nread < DATASIZE) {
            // check if available to read
            rb_read_start(cfd, &rstart, &cavailable);
            cptr = (int *)rstart;
            
            // if available read
            if (cavailable) {
                for (int i = 0; i < (cavailable / 4); i++) {
                    if (cptr[i] != rand()) {
                        missmatch++;
                    }
                    else {
                        match++;
                    }
                }
                rb_read_finish(cfd, cavailable);
                nread += cavailable;
            }
        }

        printf("%d match\t%d missmatch\n", match, missmatch);

        // close reader
        rb_close(cfd);
    }
    else {
        int available = 0;
        int nwrite = 0;
        int wchunk;
        void *wstart = 0;
        int *pptr;

        int fd;
        
        // open writer
        fd = rb_open("ring");

        while (nwrite < DATASIZE) {
            // check if space available to write
            rb_write_start(fd, &wstart, &available);
            
            pptr = (int *)wstart;
            wchunk = WCHUNK;
            
            // if available write
            if (available) {
                // how many left to write
                if ((DATASIZE - nwrite) < wchunk) {
                    wchunk = (DATASIZE - nwrite); 
                }

                for (int i = 0; i < (wchunk / 4); i++) {
                    pptr[i] = rand(); 
                }

                rb_write_finish(fd, wchunk);
                nwrite += wchunk;
            }
        }
        
        // wait for reader to finish
        wait((int *)0);

        // close the writing process
        rb_close(fd);
    }

    exit(0);
}
