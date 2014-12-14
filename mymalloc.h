#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define recP 113581321
#define memblocksize 1024*1024
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"



static char memblock[memblocksize];

struct mementry{

	struct mementry *prev, *next;
	int isFree:1; //1 bit
	int size:31; //31 bits
	int recPattern;

};



void* mymalloc(unsigned int size, const char* file, const int line);
void myfree(void* p, const char* file, const int line);
void* mycalloc(unsigned int num, unsigned int size, const char* file, const int line);
void* myrealloc(void *ptr, unsigned int size);
