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

struct page {
  void *address;
};
typedef struct page *page_t;

struct TPS {
  pthread_t tid;
  page_t page;
  struct TPS *copyFrom;
  queue_t copyingMe;
};
typedef struct TPS *tps_t;

queue_t tpsHolders;

// Compares the current target TPS address to the p_fault
int queue_address(void* targetTPS, void* address) {
  return ((tps_t) targetTPS)->page->address == address ? 1 : 0;
}

tps_t tps_address_find(void* targetTPS) {
  tps_t current = NULL;
  if (tpsHolders == NULL || targetTPS == NULL) {
    return NULL;
  }
  // Iterate through the queue and see if targetTPS exist in said queue
  // If it does not exist we return NULL
  queue_iterate(tpsHolders, queue_address, (void*)targetTPS, (void**)&current);
  if (current == NULL) {
    return NULL;
  }
  return current;
}

static void segv_handler(int sig, siginfo_t *si, void *context)
{
    /*
     * Get the address corresponding to the beginning of the page where the
     * fault occurred
     */
    void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));


    // Iterate through all the TPS areas and find if p_fault matches one of them
    tps_t targetTPS = tps_address_find(p_fault);

    // If we do find a match we print the error
    if (targetTPS != NULL)
        // Printf the following error message
        fprintf(stderr, "TPS protection error!\n");

    // In any case, restore the default signal handlers
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    // And transmit the signal again in order to cause the program to crash
    raise(sig);
}

int tps_init(int segv)
{
  tpsHolders = queue_create();


  if (segv) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segv_handler;
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
  }



  return 0;
}

// queue_iterate function to return tps with given tid
int tps_find(void* tps, void* tid){
  return (((tps_t)tps)->tid == (pthread_t)tid) ? 1 : 0;
}

int tps_create(void)
{

  // Check for tps for current thread
  tps_t current_tps = NULL;
  queue_iterate(tpsHolders, tps_find, (void*)pthread_self(), (void**)&current_tps);
  if(current_tps != NULL){
    // Return -1 if already tps for this tid
    return -1;
  }

  tps_t currTPS = (tps_t)malloc(sizeof(struct TPS));
  if (currTPS == NULL) {
    // Return -1 if malloc failed
    return -1;
  }

  currTPS->tid = pthread_self();
  page_t page = malloc(sizeof(struct page));
  if (page == NULL){
    // Return -1 if malloc failed
    return -1;
  }

  page->address = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, FD, OFFSET);
  currTPS->page = page;
  queue_enqueue(tpsHolders, currTPS);

  return 0;
}

int tps_destroy(void)
{
  // Get tps for current thread
  tps_t tps = NULL;
  queue_iterate(tpsHolders, tps_find, (void*)pthread_self(), (void**)&tps);
  if(tps == NULL){
    // Return -1 if no tps for this tid
    return -1;
  }

  // Delete tps stuff
  munmap(tps->page->address, TPS_SIZE);
  free(tps->page);
  free(tps);

  return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
  // Get tps for current thread
  tps_t tps = NULL;
  queue_iterate(tpsHolders, tps_find, (void*)pthread_self(), (void **)&tps);
  if(tps == NULL){
    // Return -1 if no tps for this tid
    return -1;
  }

  // Error if out of bounds
  if(offset + length > TPS_SIZE || offset < 0){
    return -1;
  }

  enter_critical_section();
  // Read from mem
  mprotect(tps->page->address, length, PROT_READ);
  memcpy(buffer, tps->page->address + offset, length);
  mprotect(tps->page->address, length, PROT_NONE);
  exit_critical_section();

  return 0;
}

int actually_copy(void *dest, void *source){
  tps_t tps_dest = (tps_t)dest;
  tps_t tps_source = (tps_t)source;

  // Create new page
  tps_dest->page->address = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, FD, OFFSET);

  enter_critical_section();
  // Copy tps
  mprotect(tps_dest->page->address, TPS_SIZE, PROT_WRITE);
  mprotect(tps_source->page->address, TPS_SIZE, PROT_READ);
  memcpy(tps_dest->page->address, tps_source->page->address, TPS_SIZE);
  mprotect(tps_dest->page->address, TPS_SIZE, PROT_NONE);
  mprotect(tps_source->page->address, TPS_SIZE, PROT_NONE);
  exit_critical_section();

  // Delete tps_dest from tps_source->copyingMe
  queue_delete(tps_source->copyingMe, tps_dest);

  // Set tps_dest->copyFrom to null
  tps_dest->copyFrom = NULL;

  return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
  // Get tps for current thread
  tps_t tps = NULL;
  queue_iterate(tpsHolders, tps_find, (void*)pthread_self(), (void**)&tps);
  if(tps == NULL){
    // Return -1 if no tps for this tid
    return -1;
  }

  // Error if out of bounds
  if(offset + length > TPS_SIZE || offset < 0){
    return -1;
  }

  // Copy-on-write activation for this TPS
  if(tps->copyFrom){
    actually_copy(tps,tps->copyFrom);
  }

  // Copy-on-write activation for any TPS pointing at this TPS
  int *dummy;
  if(queue_length(tps->copyingMe)>0){
    queue_iterate(tps->copyingMe, actually_copy, (void*)tps, (void**)&dummy);
  }

  enter_critical_section();

  // Write to mem
  mprotect(tps->page->address, length, PROT_WRITE);
  memcpy(tps->page->address + offset, buffer, length);
  mprotect(tps->page->address, length, PROT_NONE);
  exit_critical_section();

  return 0;
}

// Creates a TPS without a page to save memory in Copy-on-Write
int tps_create_with_pointer(tps_t tps)
{
  tps_t currTPS = (tps_t)malloc(sizeof(struct TPS));

  if (currTPS == NULL) {
    return -1;
  }

  currTPS->tid = pthread_self();
  currTPS->page = (page_t)malloc(sizeof(struct page));
  if (currTPS->page == NULL){
    return -1;
  }

  currTPS->page->address = tps->page->address;
  queue_enqueue(tpsHolders, currTPS);
  currTPS->copyFrom = tps;
  queue_enqueue(tps->copyingMe, currTPS);

  return 0;
}

int tps_clone(pthread_t tid)
{
  enter_critical_section();

  // Check for tps for current thread
  tps_t current_tps = NULL;
  queue_iterate(tpsHolders, tps_find, (void*)pthread_self(), (void**)&current_tps);
  if(current_tps != NULL){
    // Return -1 if already tps for this tid
    return -1;
  }

  // Get tps for target thread
  tps_t tps = NULL;
  queue_iterate(tpsHolders, tps_find, (void*)tid, (void**)&tps);
  if(tps == NULL){
    // Return -1 if no tps for this tid
    return -1;
  }

  tps_create_with_pointer(tps);

  exit_critical_section();
  return 0;
}
