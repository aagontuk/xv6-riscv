#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"

struct barrier {
  struct spinlock block;
  int chan;
};

struct barrier br0;

int barrierread(int user_dst, uint64 dst, int n) {
  int awakened_process = 0;
  
  acquire(&br0.block);
  br0.chan++;
  printf("chan: %d\n", br0.chan);
  sleep(&br0.chan, &br0.block);

  awakened_process = br0.chan;
  br0.chan--;
  printf("chan: %d\n", br0.chan);
  printf("awak: %d\n", awakened_process);
  release(&br0.block);
  
  return 4;
}

int barrierwrite(int user_dst, uint64 dst, int n) {
  printf("waking up\n");
  wakeup(&br0.chan);

  return 4;
}

void barrierinit(void) {
  /*
  for (int i = BR0; i <= BR9; i++) {
    devsw[i].read = barrierread;
    devsw[i].write = barrierwrite;
  }
  */
  devsw[BR0].read = barrierread;
  devsw[BR0].write = barrierwrite;
  initlock(&br0.block, "br0lock");
}
