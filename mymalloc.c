#include "mymalloc.h"
void leakDetection(){

	printf("hello from leakDetection :)\n");


}

void* mycalloc(unsigned int num, unsigned int size, const char* file, const int line){


	unsigned int totalsize = num*size;
	char* head, *ptr;
	int i;

	ptr = (char*)mymalloc(totalsize, file, line);
	head = ptr;

	for(i = 0; i<totalsize; i++){
	
		*ptr = 0;
		ptr++;

	}

	return head;


}

//mymalloc takes in an unsigned int, file, and line number. It attempts to find/create blocks of memory to return, and returns errors when needed.
void* mymalloc(unsigned int size, const char* file, const int line){

	static struct mementry *root = NULL;
	static struct mementry *small = NULL;
	struct mementry* p = NULL;

	static int initialized = 0; //use boolean to know if memblock has been initialized
	static struct mementry *prev, *next;

	//If memblock is not initialized, create first mementry in block
	if(!initialized){

		//create root mementry at beginning
		root = (struct mementry*) memblock;
		root->prev = 0;
		root->next = 0;
		root->size = memblocksize - sizeof(struct mementry);
		root->isFree = 1;
		root->recPattern = recP;

		//create small mementry at center of memblock for smaller allocations
		small = (struct mementry*) &(memblock[(3*memblocksize)/4]);
		small->prev = root;
		small->next = 0;
		root->next = small;
		root->size = root->size - (3*memblocksize)/4;
		small->size = (memblocksize/4) - sizeof(struct mementry*);
		small->isFree = 1;
		small->recPattern = recP;	

		initialized = 1;

	}

	//check if size is small. If so, start at the middle of memblock
	if(size <= 8){
		p = small;	
	}else{
		p = root;
	}


	if(size<= 8){
		do{

			if((p->size < size )){
				//current block of memory too small to allocate or
				//p is not free
				p = p->next;

			}else if(p->isFree == 0){
			
				p = p->next;

			}else if(p->size <= size + sizeof(struct mementry)){
				//there is enough size for this mementry, but not create another
				//mementry afterwords
			
				p->isFree = 0;
				return (char*) p + sizeof(struct mementry);
			

			}else{
				//enough size for mementry, put it there
				next = (struct mementry*)((char*) p + sizeof(struct mementry) + size);
				next->prev = p;
				next->next = p->next; 
	
				//check if this is the last mementry 
				if(p->next != 0){
					p->next->prev = next;
				}
		
				p->next = next;
				next->size = (p->size - sizeof(struct mementry)) - size;
				next->isFree = 1;
				p->size = size;
				p->isFree = 0;
	
				return (char*) p + sizeof(struct mementry);
			}			
			


		}while (p != 0);

		//if program gets here, no space large enough was found in the section for small allocations. set p to root and check the section for large allocations.
	}

	p = root;

	//find a block of memory big enough to allocate
	do{



		if((p->size < size )){
			//current block of memory too small to allocate or
			//p is not free
			p = p->next;


		}else if(p->isFree == 0){
			
			p = p->next;

		}else if(p->size <= size + sizeof(struct mementry)){
			//there is enough size for this mementry, but not create another
			//mementry afterwords
			
			p->isFree = 0;
			return (char*) p + sizeof(struct mementry);
			

		}else{
			//enough size for mementry, put it there
			next = (struct mementry*)((char*) p + sizeof(struct mementry) + size);
			next->prev = p;
			next->next = p->next; 

			//check if this is the last mementry 
			if(p->next != 0){
				p->next->prev = next;
			}
	
			p->next = next;
			next->size = (p->size - sizeof(struct mementry)) - size;
			next->isFree = 1;
			p->size = size;
			p->isFree = 0;

			return (char*) p + sizeof(struct mementry);
		}
	
	}while(p != 0);


	//If program has gotten here, there is no memory block big enough for that requested. Give error.
	fprintf(stderr, COLOR_RED "ERROR: Requested memory allocation at " COLOR_GREEN "line %d" COLOR_RED 
	" in" COLOR_GREEN " file %s" COLOR_RED " of" COLOR_GREEN " size %d " COLOR_RED" is too large." COLOR_RESET "\n", line, file, size);
	
	_exit(0);

}


//myfree takes in a void ponter, a file, and line number. It attempts to free the requested block of memory, throwing an error when necessary.
void myfree(void* p, const char* file, int line){


	if(p == NULL){
		//trying to free unallocated variable	
		fprintf(stderr, COLOR_RED "ERROR: Requested free at " COLOR_GREEN "line %d" COLOR_RED" in" COLOR_GREEN 
		" file %s" COLOR_RED " cannot be completed; the variable was not allocated." COLOR_RESET "\n", line, file);
		_exit(0);
	
	}

	int* intptr = p;
	intptr--;

	//check for recognition pattern that only occurs in heap variables  
	if(*intptr != 113581321){

		fprintf(stderr, COLOR_RED "ERROR: Attempt to free something that was not a pointer returned by mymalloc at " 
		COLOR_GREEN "line %d " COLOR_RED "of " COLOR_GREEN "file %s" COLOR_RED"." COLOR_RESET"\n", line, file);
		_exit(0);


	}

	struct mementry *ptr, *prev, *next;

	ptr = (struct mementry*)((char*) p - sizeof(struct mementry));


	//check if this is an attempt to free something that has already been freed
	if(ptr->isFree == -1){
		fprintf(stderr, COLOR_RED "ERROR: Attempt to free memory that has already been freed at " COLOR_GREEN "line %d " 
		COLOR_RED "of " COLOR_GREEN "file %s" COLOR_RED "." COLOR_RESET "\n", line, file);
		_exit(0);
	}

	//check if there is a previous and it is free
	if(prev = ptr->prev != 0 && prev->isFree == 1){

		//add the ptr mementry to prev->size
		prev->size += sizeof(struct mementry) + ptr->size;
		prev->next = ptr->next;

		//check if ptr is not the last mementry
		if(ptr->next != 0){
			//fix ptr->next's prev to be the current prev
			ptr->next->prev = prev;
		}

	}else{
		ptr->isFree = 1;
		prev = ptr;
	}

	if((next = ptr->next) != 0 && next->isFree == 1){
		
		prev->size += sizeof(struct mementry) + next->size;
		prev->next = next->next;

		if(next->next != 0){
			next->next->prev = prev;
		}
	}
}

#define myfree( x ) myfree( x, __FILE__, __LINE__)
#define mycalloc( x, y ) mycalloc( x, y, __FILE__, __LINE__)
#define mymalloc( x ) mymalloc( x, __FILE__, __LINE__)
int main(){

	char* catPtr = (char*)mymalloc(5);
	strcpy(catPtr, "hiss");
	printf("%s\n", catPtr);

	int* five = mymalloc(2);
	*five = 3;
	printf("%d\n", *five);


	char* dog = (char*)mymalloc(100);
	strcpy(dog, "woof");
	printf("%s\n", dog);


	//test calloc
	
	char* bird = (char*)mycalloc(3, 12);

	strcpy(bird, "chirp");
	printf("%s\n", bird);





}

