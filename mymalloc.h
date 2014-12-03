#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char memblock[5000];

struct mementry{

	struct mementry *prev, *next;
	int isFree:1; //1 bit
	int size:31; //31 bits

};

void* mymalloc(unsigned int size);

void myfree(void* p);
