#include "libring.h"
#include "kernel/types.h"
#include "kernel/param.h"
#include "user.h"

#define SIZE (RINGBUF_SIZE * 4096)

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

// find a ring by name
int find_ring(char *name) {
    for (int i = 0; i < NPROC; i++) {
        
        if (urings[i].exist != 0 &&
              strcmp(urings[i].name, name) == 0) {
            
            return i;

       }
    }

    return -1;
}

// open a ring buffer
// return the discriptor of the buffer
int rb_open(char *name) {
    struct book *b;

    for (int i = 0; i < NPROC; i++) {
        
        if (!urings[i].exist) {
            urings[i].exist = 1;
            
            // create a new ring buffer
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

// close ring
int rb_close(int desc) {
    int ret = ringbuf(urings[desc].name, 1, (uint64)&urings[desc].buf);
    
    urings[desc].exist = 0;
    strcpy(urings[desc].name, "");

    return ret;
}

void rb_write_start(int desc, void **addr, int *bytes) {
    struct book *b = (struct book *)urings[desc].book;
    
    int writep = __atomic_load_8(&b->writep, __ATOMIC_SEQ_CST);
    int readp = __atomic_load_8(&b->readp, __ATOMIC_SEQ_CST);
    int wavail;

    // end of buffer
    if (writep != 0 && writep % SIZE == 0) {
        wavail = SIZE;    
    }
    // in the middle of buffer
    else {
        wavail = writep % SIZE; 
    }

    *bytes = ((SIZE - wavail) + (readp % SIZE));
    *addr = urings[desc].buf + (writep % SIZE);
}

void rb_write_finish(int desc, int bytes) {
    struct book *b = (struct book *)urings[desc].book;
    int curw = __atomic_load_8(&b->writep, __ATOMIC_SEQ_CST);    
    
    __atomic_store_8(&b->writep, curw + bytes, __ATOMIC_SEQ_CST);
}

void rb_read_start(int desc, void **addr, int *bytes) {
    struct book *b = (struct book *)urings[desc].book;
    int writep = __atomic_load_8(&b->writep, __ATOMIC_SEQ_CST);
    int readp = __atomic_load_8(&b->readp, __ATOMIC_SEQ_CST);

    *bytes = writep - readp;
    *addr = urings[desc].buf + (readp % SIZE);
}

void rb_read_finish(int desc, int bytes) {
    struct book *b = (struct book *)urings[desc].book;
    int curr = __atomic_load_8(&b->readp, __ATOMIC_SEQ_CST);    
    
    __atomic_store_8(&b->readp, curr + bytes, __ATOMIC_SEQ_CST);
}
