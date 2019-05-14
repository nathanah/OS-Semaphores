will write later
#Semaphore
##structure
Each semaphore consists of just a count variable and a queue pointer.

##functions


#TPS
##structure
Each TPS contains the tid for the thread it belongs to, a tid for if this TPS is copying from another, a queue for all TPSs that are copying from this TPS, and our page struct.

##functions
We use helper functions actually_copy() and tps_find() in additions to the functions defined in tps.h to simplify our code. tps_find() allows for us to use queue_iterate() to find the TPS that corresponds to a given tid. actually_copy() is used for the copy-on-write cloning.


##Copy-on-Write Cloning
To allow for copy-on-write cloning, instead of actually copying the page in tps_clone, we make the TPS copy point to the same page until there is a write. This can happen with several copies, and they would all need to actually copy given either them or the source was written to. This is why we use the copyFrom variable and copyingMe queue. When there is a write to a page, it will first copy the page pointed to by copyFrom, then copy to all elements in copyingMe before writing to its own page. This process uses the actually_copy() method instead of tps_clone() so it is compatible with queue_iterate() and can copy to pages that do not belong to the current running thread.

##testing
..
