#include "kernel/types.h"
#include "user/user.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[]) {
  int fd0, pid;
  int nchild = 5;
  int status;

  fd0 = open("barrier0", O_RDWR);
  
  for (int i = 0; i < nchild; i++) {
    pid = fork(); 
    if (!pid) {
      read(fd0, (int *)0, 0);
      exit(i);
    }
  }

  if (pid != 0) {
    for (int i = 0;;i++) {
      if (i >= 2000000) 
        break;
    }
    write(fd0, (int *)0, 0);
    
    for (int i = 0; i < nchild; i++) {
      wait(&status);
      printf("child %d exited!\n", status);
    }
  }

  exit(0);
}

