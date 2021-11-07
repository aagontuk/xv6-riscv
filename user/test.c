#include "kernel/types.h"
#include "user/user.h"

int main() {
  int pid;

  pid = fork();

  if (pid == 0) {
    printf("hello from child!\n");  
  }
  else {
    printf("hello from parent!\n");
    wait((int *)0); 
  }

  exit(0);
}
