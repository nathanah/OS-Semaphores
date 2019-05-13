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
  void *address;
}
typedef struct page *page_t;

struct TPS {
  pthread_t TID;
  struct pageHandler *page;
}
typedef struct TPS *tps_t;

queue_t tpsHolders;


static void segv_handler(int sig, siginfo_t *si, void *context)
{
    /*
     * Get the address corresponding to the beginning of the page where the
     * fault occurred
     */
    void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));

    /*
     * Iterate through all the TPS areas and find if p_fault matches one of them
     */
    ...
    if (/* There is a match */)
        /* Printf the following error message */
        fprintf(stderr, "TPS protection error!\n");

    /* In any case, restore the default signal handlers */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    /* And transmit the signal again in order to cause the program to crash */
    raise(sig);
}

int tps_init(int segv)
{
	/* TODO: Phase 2 */
  tpsHolders = queue_create();

  //...
  if (segv) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segv_handler;
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
  }
  //...


  return 0;
}

int tps_create(void)
{
	/* TODO: Phase 2 */
  tps_t currTPS = (tps_t)malloc(sizeof(struct TPS));

  if (!currTPS) {
    return -1;
  }

  currTPS->TID = pthread_self();
  page_t page = malloc(sizeofstruct(page));
  if (!page)
    return 1;

  page->address = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, FD, OFFSET);
  currTPS->page = page;
  queue_enqueue(tpsHolders, currTPS);

  return 0;
}

//queue_iterate function to return tps with given tid
int tps_find(tps_t tps, pthread_t tid){
  if(tps->tid == tid){
    return 1;
  }
  else
    return 0;
}

int tps_destroy(void)
{
	/* TODO: Phase 2 */
  //get tps for current thread
  tps_t tps;
  if(queue_iterate(tpsHolders, tps_find, pthread_self(), &tps) == -1){
    //return -1 if no tps for this tid
    return -1;
  }

  //delete tps stuff
  munmap(tps->page->address, TPS_SIZE);
  free(tps);

  return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	/* TODO: Phase 2 */
  //get tps for current thread
  tps_t tps;
  if(queue_iterate(tpsHolders, tps_find, pthread_self(), &tps) == -1){
    //return -1 if no tps for this tid
    return -1;
  }

  //Read from mem
  mprotect(tps->page->address, length, PROT_READ);
  memcpy(buffer, tps->page->address + offset, length);
  mprotect(tps->page->address, length, PROT_NONE);

  return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	/* TODO: Phase 2 */
  //get tps for current thread
  tps_t tps;
  if(queue_iterate(tpsHolders, tps_find, pthread_self(), &tps) == -1){
    //return -1 if no tps for this tid
    return -1;
  }

  //Write to mem
  mprotect(tps->page->address, length, PROT_WRITE);
  memcpy(tps->page->address + offset, buffer, length);
  mprotect(tps->page->address, length, PROT_NONE);

  return 0;
}

int tps_clone(pthread_t tid)
{
	/* TODO: Phase 2 */
  //check for tps for current thread
  tps_t current_tps;
  if(queue_iterate(tpsHolders, tps_find, pthread_self(), &current_tps) == 0){
    //return -1 if already tps for this tid
    return -1;
  }
  //get tps for target thread
  tps_t tps;
  if(queue_iterate(tpsHolders, tps_find, tid, &tps) == -1){
    //return -1 if no tps for this tid
    return -1;
  }

  //Create new tps
  tps_create();
  if(queue_iterate(tpsHolders, tps_find, pthread_self(), &current_tps) == -1){
    //return -1 if no tps for this tid
    return -1;
  }

  //Copy tps
  mprotect(current_tps->page->address, TPS_SIZE, PROT_WRITE);
  mprotect(tps->page->address, TPS_SIZE, PROT_READ);
  memcpy(current_tps->page->address, current_tps->page->address, TPS_SIZE);
  mprotect(current_tps->page->address, TPS_SIZE, PROT_NONE);
  mprotect(tps->page->address, TPS_SIZE, PROT_NONE);

  return 0;
}
