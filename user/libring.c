#include "libring.h"
#include "kernel/types.h"
#include "kernel/param.h"
#include "user.h"

struct uring {
    char name[MAX_RING_NAME];
    void *buf;
    void *book;
    int exist;
};

struct book {
    uint64 readp;
    uint64 writep;
};

struct uring urings[NPROC];

int find_ring(char *name) {
    for (int i = 0; i < NPROC; i++) {
        if (!strcmp(urings[i].name, name)) 
            return i;
    }

    return -1;
}

int rb_open(char *name) {
    struct book *b;

    for (int i = 0; i < NPROC; i++) {
        if (!urings[i].exist) {
            urings[i].exist = 1;
            
            ringbuf(name, 0, (uint64)&urings[i].buf);
            urings[i].book = urings[i].buf + (4096 * (RINGBUF_SIZE * 2));
            
            // initialize bookkepping if called for the first time
            if (find_ring(name) < 0) {
                b = (struct book *)urings[i].book;
                b->readp = 0;
                b->writep = 0;
            }
            
            strcpy(urings[i].name, name);

            return i;
        }
    }

    return -1;
}

int rb_close(int desc) {
    urings[desc].exist = 0;
    return ringbuf(urings[desc].name, 1, (uint64)&urings[desc].buf);
}

void bookr(int desc) {
    struct book *b = (struct book *)urings[desc].book;
    printf("writep: %d\treadp: %d\n", b->writep, b->readp);
}

void bookw(int desc) {
    char *str = (char *)urings[desc].book;
    strcpy(str, "riscv");
}

void rb_write_start(int desc, char **addr, int *bytes) {

}

void rb_write_finish(int desc, int bytes) {

}

void rb_read_start(int desc, char **addr, int *bytes) {

}

void rb_read_finish(int desc, int bytes) {

}
