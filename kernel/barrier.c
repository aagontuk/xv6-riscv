#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"

struct barrier {
  struct spinlock lock;
  int nsleeping;
};

struct barrier br[NBDEV]; // NBDEV in param.h

int barrierread(short minor, int user_dst, uint64 dst, int n) {
  // Only reading 4 bytes is allowed
  if (n != 4)
    return -1;
  
  acquire(&br[minor].lock);
  br[minor].nsleeping++;
  sleep(&br[minor].nsleeping, &br[minor].lock);

  // copy number of awakened processes into user buffer
  if (either_copyout(user_dst, dst, &br[minor].nsleeping, 4) == -1) {
    br[minor].nsleeping--; 
    release(&br[minor].lock);
    return -1;
  }

  br[minor].nsleeping--;
  release(&br[minor].lock);
  
  return 4;
}

int barrierwrite(short minor, int user_dst, uint64 dst, int n) {
  // Only writing 4 bytes is allowed
  if (n != 4)
    return -1;

  // wakeup all the process currently sleeping on minor barrier
  wakeup(&br[minor].nsleeping);

  return 4;
}

void barrierinit(void) {
  devsw[BARRIER].read = barrierread;
  devsw[BARRIER].write = barrierwrite;
  
  // initialize locks for all the devices
  char *names[] = {"brlk0", "brlk1", "brlk2", "brlk3", "brlk4",
                   "brlk5", "brlk6", "brlk7", "brlk8", "brlk9"};
  
  for (int i = 0; i < NBDEV; i++) {
    initlock(&br[i].lock, names[i]);
  }
}
