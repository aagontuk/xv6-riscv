#include "kernel/types.h"
#include "user/user.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[]) {
  int fd0, fd1;
  int pid;
  int nchild = 5;
  int status;
  int nawake;

  fd0 = open("barrier0", O_RDWR);
  fd1 = open("barrier1", O_RDWR);

  // five new process each calling read on barrier0
  for (int i = 0; i < nchild; i++) {
    pid = fork(); 
    if (!pid) {
      read(fd0, &nawake, 4);
      printf("%d\n", nawake);
      exit(i);
    }
  }

  // five more child to read from barrier1
  if (pid != 0) {
    for (int i = 0; i < nchild; i++) {
      pid = fork(); 
      if (!pid) {
        read(fd1, &nawake, 4);
        printf("%d\n", nawake);
        exit(i);
      }
    }
  }

  if (pid != 0) {
    // To make sure all the reads are done
    for (int i = 0;;i++) {
      if (i >= 2000000) 
        break;
    }

    // wakeup all child sleeping on barrier0
    write(fd0, (int *)0, 4);

    // wakeup all child sleeping on barrier1
    write(fd1, (int *)0, 4);
    
    // wait for all the child to exit
    for (int i = 0; i < (nchild * 2); i++) {
      wait(&status);
      printf("child %d exited!\n", status);
    }
  }

  exit(0);
}

