#include "kernel/types.h"
#include "user/user.h"
#include "user/libring.h"

#define DATASIZE (4096*32)
#define CHUNKSIZE 1024

void str_test(uint64 addr) {
    char *s = (char *)addr;
    char *t = s + (4096*16);
    char *u = s + 4096;
    char *v = s + (4096*17);

    printf("\n\nstring test:\n\n");

    strcpy(s, "xv6");
    strcpy(u, "riscv");

    printf("first pair: (va+0, va+(4096*16))\n");
    printf("s:\taddr:%p\tval:%s\n", s, s);
    printf("t:\taddr:%p\tval:%s\n", t, t);
    
    
    printf("\nsecond pair: (va+4096, va+(4096*17))\n");
    printf("u:\taddr:%p\tval:%s\n", u, u);
    printf("v:\taddr:%p\tval:%s\n", v, v);
}

void int_test(uint64 addr) {
    int *x = (int *)addr;
    int *y = (int *)(addr + (4096*16));

    int *w = (int *)(addr + 4096);
    int *v = (int *)(addr + (4096*17));

    printf("\n\ninteger test:\n\n");
    
    *x = 10;
    *w = 100;
    
    printf("first pair: (va+0, va+(4096*16))\n");
    printf("addr: %p\tval:%d\n",x, *x);
    printf("addr: %p\tval:%d\n", y, *y);
    
    printf("\nsecond pair: (va+4096, va+(4096*17))\n");
    printf("addr: %p\tval:%d\n",w, *w);
    printf("addr: %p\tval:%d\n", v, *v);
}

void rbwrite(void *addr) {
    strcpy((char *)addr, "UTAH");
}

void rbread(void *addr) {
    printf("%s\n", (char *)addr);
}

int main(void) {
    //void *addr;
    //int ret;
    int parent_fd;
    int pid; 
    int child_open_failed = 0;
    int child_close_failed = 0;
    
    if ((parent_fd = rb_open("ring")) < 0) {
        printf("Failed opening parent!\n"); 
    }

    pid = fork();
    
    if (pid == 0) {
        int child_fd;
        int written = 0;
        int write_size = 0;
        int available;
        int j = 0;
        void *data;

        if ((child_fd = rb_open("ring")) < 0) {
            child_open_failed = 1;
        }

        while (written < DATASIZE) {
            rb_write_start(child_fd, &data, &available);
            int *iptr = (int *)data;
            if (available) {
               if (available > CHUNKSIZE) {
                   write_size = CHUNKSIZE;
               }
               else {
                   write_size = available; 
               }

               for (int i = 0; i < (write_size / 4); i++) {
                   *(iptr + i) = j++;
               }
               
               rb_write_finish(child_fd, write_size);
               written += write_size;
            }
        }
        
        if ((rb_close(child_fd)) < -1) {
            child_close_failed = 1;
        }
    }
    else {
        void *data;
        int available;
        int read = 0;
        int j = 0;
        int match = 0;
        int missmatch = 0;

        while (read < DATASIZE) {
            rb_read_start(parent_fd, &data, &available);
            int *iptr = (int *)data;
            if (available) {
                for (int i = 0; i < (available / 4); i++) {
                    if (*(iptr + i) != j++) {
                        missmatch++;
                        printf("data missmatch! %d\n", j);
                    }
                    else {
                        match++;
                        //printf("data match! %d\n", match);
                    }
                } 
                rb_read_finish(parent_fd, available);
                read += available;
            }
        }

        wait((int *)0);

        printf("%d match and %d missmatch!\n", match, missmatch);
        
        if ((rb_close(parent_fd)) < -1) {
            printf("Failed closing parent!\n"); 
        }

        if (child_open_failed) {
            printf("failed opening child!\n");
        }

        if (child_close_failed) {
            printf("failed closing child!\n");
        }
    }
    
    exit(0);
}
