CC = gcc
CCFLAGS = -g

all: MallocTest

MallocTest: mymalloc.c
		$(CC) $(CCFLAGS) -o MallocTest mymalloc.c

clean:
	-rm -f *.o
	-rm -f MallocTest
