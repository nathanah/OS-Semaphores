#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>

void *thread1 (void *arg) {
  char str[1000] = {0};
  assert(tps_write(0, 1000, str) == -1)
  
}
