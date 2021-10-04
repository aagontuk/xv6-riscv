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

struct uring urings[NPROC];

int rb_open(char *name) {
    for (int i = 0; i < NPROC; i++) {
        if (!urings[i].exist) {
            strcpy(urings[i].name, name);
            urings[i].exist = 1;
            
            ringbuf(name, 0, (uint64)&urings[i].buf);

            urings[i].book = urings[i].buf + (4096 * (RINGBUF_SIZE * 2));

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
    printf("%s\n", (char *)urings[desc].book);
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
