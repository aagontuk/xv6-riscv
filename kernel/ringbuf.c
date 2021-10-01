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
    void *buf[RINGBUF_SIZE];
    void *book;
};


struct spinlock ringbuf_lock; // Will use it later
struct ringbuf ringbufs[MAX_RINGBUFS];

int strcmp(const char *, const char *);
char *strcpy(char *, const char *);

int create_ringbuf(char *name, int type, uint64 addr) {
    struct ringbuf new_ring;

    // already exists in the pool
    for (int i = 0; i < MAX_RINGBUFS; i++) {
        if (!strcmp(ringbufs[i].name, name)) {
            ringbufs[i].refcount++; 
            printf("Already exists: %d\n", ringbufs[i].refcount);
            return -1;
        }   
    }

    // create a new one
    strcpy(new_ring.name, name);
    new_ring.refcount = 1;

    // allocate physical pages
    for (int i = 0; i <= RINGBUF_SIZE; i++) {
        if (i != RINGBUF_SIZE)
            new_ring.buf[i] = kalloc(); 
        else
            new_ring.book = kalloc();
    }

    // map pages into process's address space
    struct proc *p = myproc();
    printf("pid: %d\n", p->pid);

    acquire(&p->lock);
    if (mappages(p->pagetable, PGSIZE * 1000, PGSIZE, (uint64)new_ring.buf[0], PTE_U | PTE_R | PTE_W | PTE_X) < 0) {
        printf("mappage() failed\n");
        release(&p->lock);
        return -1;
    }
    release(&p->lock);

    // find place in the pool
    ringbufs[0] = new_ring;

    printf("New ring buffer created!\n");

    return 0;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}
