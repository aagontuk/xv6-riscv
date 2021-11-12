#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"

unsigned long rdcycle(void) {
  unsigned long dst;
  asm volatile ("csrrs %0, 0xc00, x0" : "=r" (dst));
  return dst;
}

// Read 8 byte from csrrs reg
int cycleread(short minor, int user_dst, uint64 dst, int n) {
  // cycle will only read 8 bytes
  if (n != 8)
    return -1;

  unsigned long cycle = rdcycle();
  
  // This device can be used both in kernel and user space
  if (either_copyout(user_dst, dst, &cycle, 8) == -1) 
    return -1;
  
  return 8;
}

// Write is not allowed
// This is only here to fail
int cyclewirte(short monor, int user_dst, uint64 dst, int n) {
  return -1;
}

void cycleinit(void) {
  devsw[CYCLE].read = cycleread;
  devsw[CYCLE].write = cyclewirte;
}
