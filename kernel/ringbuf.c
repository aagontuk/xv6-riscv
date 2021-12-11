#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

// MAXVA - 1GB
#define MAP_START (MAXVA - (1024*1024*1024))

struct ringbuf {
    int refcount;
    char name[MAX_RING_NAME];
    void *pages[RINGBUF_SIZE];
    void *book;
};


int init_ringbuf = 1;
struct spinlock ringbuf_lock;
struct ringbuf ringbufs[MAX_RINGBUFS];


// check if the ring buffer already exists.
// If exists return the ring buffer
// otherwise return 0
struct ringbuf *find_ring(char *name) {
    struct ringbuf *rb;

    acquire(&ringbuf_lock);
    for (rb = ringbufs; rb < &ringbufs[MAX_RINGBUFS]; rb++) {
        if (!strncmp(rb->name, name, strlen(name))) {
            release(&ringbuf_lock);
            return rb;
        }   
    }
    release(&ringbuf_lock);

    return 0;
}


// Find an empty ringbuf from the list
// Allocate physical pages for the buffer
// and bookkepping page.
struct ringbuf *allocate_ring(char *name) {
    struct ringbuf *rb;
    void **p;

    acquire(&ringbuf_lock);
    for (rb = ringbufs; rb < &ringbufs[MAX_RINGBUFS]; rb++) {
        if (!rb->refcount) {
            // copy name
            strncpy(rb->name, name, strlen(name));

            // allocate physical pages
            for (p = rb->pages; p < &rb->pages[RINGBUF_SIZE]; p++) {
                if ((*p = kalloc()) == 0) {
                    release(&ringbuf_lock);
                    return 0;
                } 
            }

            // allocate bookkepping page
            if ((rb->book = kalloc()) == 0) {
                release(&ringbuf_lock);
                return 0; 
            }

            // increment count
            rb->refcount++;

            release(&ringbuf_lock);
            return rb;
        }
    }

    release(&ringbuf_lock);
    return 0;
}

// create/destroy a ring buffer
// type: 0 -> create, 1 -> destroy
int create_ringbuf(char *name, int type, uint64 *addr) {
    struct ringbuf *rb;
    struct proc *p = myproc();
    void **pg;
    uint64 a = MAP_START;
    int nmap = 2; // map each phy pages 2 times

    if (init_ringbuf) {
        initlock(&ringbuf_lock, "ring_lock");
        init_ringbuf = 0;
    }

    if (type) {
        rb = find_ring(name);
        rb->refcount--;

        /*
        if (!rb->refcount) {
            // ref = 0, remove from calling process and free phy pages
            // FIXME: Freeing physical memory isn't working if mapped
            // twice.
            uvmunmap(p->pagetable, MAP_START, (RINGBUF_SIZE * 2) + 1, 0); 
        }
        else {
            // only remove from calling process
            uvmunmap(p->pagetable, MAP_START, (RINGBUF_SIZE * 2) + 1, 0); 
        }
        */
        uvmunmap(p->pagetable, MAP_START, (RINGBUF_SIZE * 2) + 1, 0); 

        return 0;
    }

    // already exists
    // increment and map pages to calling processes address space
    if ((rb = find_ring(name)) && rb->refcount != 0) {
        // currently not supporting more than two process
        if (rb->refcount > 1) {
            return -1; 
        }

        rb->refcount++;
    }
    // allocate new ringbuf
    else {
        if ((rb = allocate_ring(name)) == 0){
            return -1; 
        }
    }

    /*
    int i = 0;
    while(nmap--) {
        for (pg = rb->pages; pg < &rb->pages[RINGBUF_SIZE]; pg++) {
            if (mappages(p->pagetable, a,
                PGSIZE, (uint64)(*pg), PTE_U | PTE_R | PTE_W) < 0) {
                
                printf("mappage() failed\n");
                return -1;
            }
            printf("PAGE%d -> %p\n",i, a);
            a += PGSIZE;
            i++;
        }
    }
    
    uvmunmap(p->pagetable, MAP_START, 32, 0); 
    rb->refcount--;
    */

    acquire(&ringbuf_lock);
    while (nmap--) {
        for (pg = rb->pages; pg < &rb->pages[RINGBUF_SIZE]; pg++) {
            if (mappages(p->pagetable, a,
                PGSIZE, (uint64)(*pg), PTE_U | PTE_R | PTE_W) < 0) {
                
                printf("mappage() failed\n");
                release(&ringbuf_lock);
                return -1;
            }
            a += PGSIZE;
        }
    }
    if (mappages(p->pagetable, a, PGSIZE, (uint64)rb->book,
            PTE_U | PTE_R | PTE_W) < 0) {
    
        printf("mappage() failed\n");
        release(&ringbuf_lock);
        
        return -1;
    }
    release(&ringbuf_lock);
    
    *addr = MAP_START;

    return 0;
}
