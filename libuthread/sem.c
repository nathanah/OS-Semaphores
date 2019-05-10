#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
	/* TODO Phase 1 */
  size_t count;
  queue_t blocked;
};

sem_t sem_create(size_t count)
{
	/* TODO Phase 1 */
  sem_t new_sem;
  new_sem = (sem_t)malloc(sizeof(semaphore));
  new_sem->blocked = queue_create();

  new_sem->count = count;
  return new_sem;
}

int sem_destroy(sem_t sem)
{
	/* TODO Phase 1 */
  if(sem == NULL || sem->blocked.size() > 0){
    return -1;
  }

  free(sem->blocked);
  free(sem);
  return 0;
}

int sem_down(sem_t sem)
{
	/* TODO Phase 1 */
}

int sem_up(sem_t sem)
{
	/* TODO Phase 1 */
}

int sem_getvalue(sem_t sem, int *sval)
{
	/* TODO Phase 1 */
}
