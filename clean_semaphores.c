#include <errno.h>
#include <semaphore.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (sem_unlink(argv[i]) != 0) {
      fprintf(stderr, "%s: ", argv[i]);
      perror("");
    }
  }
}