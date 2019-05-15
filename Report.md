# Semaphore

## structure
Each semaphore consists of just a count variable and a queue pointer.

## functions
We needed to put sem_up() and sem_down() fully into critical sections, so that
a blocked thread would immediately be resumed as soon as a sem_up is called.
This also ensures mutual exclusion such that multiple threads will be released
in the correct order. No if() blocks can be entered by concurrently running
threads at the same time based on information that one thread would change.

# TPS

## structure
Each TPS contains the tid for the thread it belongs to, a tid for if this TPS
is copying from another, a queue for all TPSs that are copying from this TPS,
and our page struct.

## functions
We use helper functions actually_copy() and tps_find() in additions to the
functions defined in tps.h to simplify our code. tps_find() allows for us to
use queue_iterate() to find the TPS that corresponds to a given tid.
actually_copy() is used for the copy-on-write cloning.

## Protected TPS
To recognize when a segfault was on a TPS, we used the provided signal handler
skeleton and added tps_address_find() and queue_address(). We would use
queue_iterate() to test if each TPS in the global queue had an page with an
address that matched where the segfault was. If there was a match, then we
would print the "TPS Protection error" message before exiting. 


## Copy-on-Write Cloning
To allow for copy-on-write cloning, instead of actually copying the page in
tps_clone, we make the TPS copy point to the same page until there is a write.
This can happen with several copies, and they would all need to actually copy
given either them or the source was written to. This is why we use the copyFrom
variable and copyingMe queue. When there is a write to a page, it will first
copy the page pointed to by copyFrom, then copy to all elements in copyingMe
before writing to its own page. This process uses the actually_copy() method
instead of tps_clone() so it is compatible with queue_iterate() and can copy to
pages that do not belong to the current running thread.

## testing
We separated the testing of our TPS into three files:
 1. tps.c (as given)
 2. tpsPt2.c
 3. tpsPt3.c
tps.c tests functionality given correct inputs while tpsPt2 and tpsPt3 test
edge cases for incorrect input or ordering.
Specifically tpsPt2 checks to see any edge cases for our tps functions.
tpsPt3 checks to ensure that our tps are protected, and threads do not 
access other thread's TPS illegally. 
