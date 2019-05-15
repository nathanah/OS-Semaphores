#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
  size_t count;
  queue_t blocked;
};

sem_t sem_create(size_t count)
{
  sem_t new_sem;
  new_sem = (sem_t)malloc(sizeof(struct semaphore));
  new_sem->blocked = queue_create();

  new_sem->count = count;
  return new_sem;
}

int sem_destroy(sem_t sem)
{
  if(sem == NULL || queue_length(sem->blocked) > 0){
    return -1;
  }

  queue_destroy(sem->blocked);
  free(sem);
  return 0;
}

int sem_down(sem_t sem)
{
  enter_critical_section();
  if(sem == NULL)
    return -1;

  while(sem->count == 0){
    queue_enqueue(sem->blocked, (void*)pthread_self());
    thread_block();
  }

  sem->count -= 1;
  exit_critical_section();
  return 0;
}

int sem_up(sem_t sem)
{
  enter_critical_section();
  if(sem == NULL)
    return -1;

  sem->count += 1;
  if (queue_length(sem->blocked) != 0) {
    void* tid;
    queue_dequeue(sem->blocked, &tid);
    thread_unblock((pthread_t)tid);
  }

  exit_critical_section();
  return 0;
}

int sem_getvalue(sem_t sem, int *sval)
{
  if(sem == NULL || sval == NULL)
    return -1;

  if (sem->count > 0) {
	  *sval = sem->count;
  }
  else {
	  *sval = -1*queue_length(sem->blocked);
  }
  //count should be the negative of the queue length by default (from sem_down count-- placement)

  return 0;
}
