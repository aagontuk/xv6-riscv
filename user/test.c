#include "kernel/types.h"
#include "user/user.h"

int main() {
  int pid;
  int nchild = 5;
  int status;

  for (int i = 0; i < nchild; i++) {
    pid = fork(); 
    if (!pid) {
      printf("I am child %d\n", i);
      exit(i);
    }
  }

  if (pid != 0) {
    printf("I am parent!\n"); 
    for (int i = 0; i < nchild; i++) {
      wait(&status);
      printf("child %d exited!\n", status);
    }
  }

  exit(0);
}
