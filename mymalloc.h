#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define recP 113581321

static char memblock[5000];

struct mementry{

	struct mementry *prev, *next;
	int isFree:1; //1 bit
	int size:31; //31 bits
	int recPattern;

};

void* mymalloc(unsigned int size, const char* file, const int line);

void myfree(void* p, const char* file, const int line);
