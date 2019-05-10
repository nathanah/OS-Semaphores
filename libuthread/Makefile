# Target library
lib := libuthread.a

# Target files of library
parts := \
	sem.o \
	tps.o

all: $(lib) $(parts)

sem.o: sem.c
	gcc -c -Wall -Werror queue.c

tps.o: tps.c
	gcc -c -Wall -Werror context.c

libuthread.a: queue.o thread.o sem.o tps.o
	ar rcs $(lib) queue.o thread.o sem.o tps.o

clean:
	rm sem.o tps.o *.a
