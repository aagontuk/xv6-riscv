#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

#define RINGBUF_SIZE 16
#define MAX_RINGBUFS 10
#define MAP_START 128*1024*1024

struct ringbuf {
    int refcount;
    char name[MAX_RING_NAME];
    void *pages[RINGBUF_SIZE];
    void *book;
};


struct spinlock ringbuf_lock; // Will use it later
struct ringbuf ringbufs[MAX_RINGBUFS];

// Check if the ring buffer already exists.
// If exists return the ring buffer
// otherwise return 0
struct ringbuf *find_ring(char *name) {
    struct ringbuf *rb;

    for (rb = ringbufs; rb < &ringbufs[MAX_RINGBUFS]; rb++) {
        if (!strncmp(rb->name, name, strlen(name))) {
            return rb;
        }   
    }

    return 0;
}


// Find an empty ringbuf from the list
// Allocate physical pages for the buffer
// and bookkepping page.
struct ringbuf *allocate_ring(char *name) {
    struct ringbuf *rb;
    void **p;

    for (rb = ringbufs; rb < &ringbufs[MAX_RINGBUFS]; rb++) {
        if (!rb->refcount) {
            // copy name
            strncpy(rb->name, name, strlen(name));

            // allocate physical pages
            for (p = rb->pages; p < &rb->pages[RINGBUF_SIZE]; p++) {
                *p = kalloc(); 
            }

            // allocate bookkepping page
            rb->book = kalloc();

            // increment count
            rb->refcount = 1;

            return rb;
        }
    }

    return 0;
}

int create_ringbuf(char *name, int type, uint64 *addr) {
    struct ringbuf *rb;
    void **pg;
    uint64 a = MAP_START;
    int nmap = 2;

    // already exists
    // increment and map pages to calling processes address space
    if ((rb = find_ring(name))) {
        printf("Already exists!\n");
        return -1;
    }

    // allocate new ringbuf
    rb = allocate_ring(name);

    // map pages into process's address space
    struct proc *p = myproc();
    printf("pid: %d\n", p->pid);

    acquire(&p->lock);
    while (nmap--) {
        for (pg = rb->pages; pg < &rb->pages[RINGBUF_SIZE]; pg++) {
            if (mappages(p->pagetable, a,
                PGSIZE, (uint64)(*pg), PTE_U | PTE_R | PTE_W) < 0) {
                
                printf("mappage() failed\n");
                release(&p->lock);
                return -1;
            }
            a += PGSIZE;
        }
    }
    release(&p->lock);
    *addr = MAP_START;

    printf("New ring buffer created!\n");

    return 0;
}