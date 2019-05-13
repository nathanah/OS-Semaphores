#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "queue.h"
#include "thread.h"
#include "tps.h"

#define FD -1
#define OFFSET 0

/* TODO: Phase 2 */
struct page
}
  void *address
}
typedef struct page *page_t

struct TPS {
  pthread_t TID;
  struct pageHandler *page
}
typedef struct TPS *tps_t;

queue_t tpsHolders;

int tps_init(int segv)
{
	/* TODO: Phase 2 */
}

int tps_create(void)
{
	/* TODO: Phase 2 */
  tps_t currTPS = (tps_t)malloc(sizeof(struct TPS));
  if !(currTPS) {
    return -1;
  }
  currTPS->TID = pthread_self();
  page_t page = malloc(sizeofstruct(page));
  if !(page)
    return 1;
  page->address = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, FD, OFFSET);
  currTPS->page = page;
  queue_enqueue(tpsHolders, currTPS);
}

int tps_destroy(void)
{
	/* TODO: Phase 2 */
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	/* TODO: Phase 2 */
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	/* TODO: Phase 2 */
}

int tps_clone(pthread_t tid)
{
	/* TODO: Phase 2 */
}
