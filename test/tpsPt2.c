#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>

void *thread1 (void *arg) {
  char str[1000] = {0};
  fprintf(stdout, "start tests\n");

  //no tps
  assert(tps_write(0, 1000, str) == -1);
  assert(tps_read(0, 1000, str) == -1);
  fprintf(stdout, "no init tests: passed\n");


  //Test double create
  assert(tps_create() == 0);
  assert(tps_create() == -1);
  fprintf(stdout, "double create: passed\n");

  //Test double destroy
  assert(tps_destroy() == 0);
  assert(tps_destroy() == -1);
  fprintf(stdout, "double destroy: passed\n");

  //Read and write out of bounds
  assert(tps_write(5000, 1000, str) == -1);
  assert(tps_read(5000, 1000, str) == -1);
  fprintf(stdout, "out of bounds: passed\n");

  fprintf(stdout, "All tests passed!\n");
  return 0;
}

int main (int argc, char **argv) {
  pthread_t tid;
  assert(tps_init(1) == 0);
  pthread_create(&tid, NULL, thread1, NULL);
  pthread_join(tid, NULL);
  return 0;
}
