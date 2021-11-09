#include "kernel/types.h"
#include "user/user.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"


int main(int argc, char *argv[]) {
  int fd = 0;
  unsigned long cycle;
  
  if ((fd = open("cycle", O_RDONLY)) < 0) {
    printf("Can't open device for reading!\n"); 
    exit(-1);
  }
  printf("cycle opened\n"); 

  if (read(fd, &cycle, 8) < 0) {
    printf("Read failed!\n");
  }
  
  printf("cycle: %l\n", cycle);
  
  if (write(fd, (int *)0, 0) < 0) {
    printf("Write failed!\n");
  }

  close(fd);

  exit(0);
}
