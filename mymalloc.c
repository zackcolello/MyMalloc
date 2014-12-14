#include "mymalloc.h"

void* myrealloc(void *ptr, unsigned int size){

	if(size == 0){
		myfree(ptr, __FILE__, __LINE__);
		return NULL;
	}
	if(ptr == NULL){
		return mymalloc(size, __FILE__, __LINE__);
	}

	int* intptr = ptr;
	intptr--;


	char* temp = (char*)mymalloc(size, __FILE__, __LINE__);
	
	struct mementry *p = NULL;

	p = (struct mementry*)((char*) ptr - sizeof(struct mementry));

	memcpy(temp, ptr, p->size);
	
	myfree(ptr, __FILE__, __LINE__);

	return temp;


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
				next->recPattern = recP;

				return (char*) p + sizeof(struct mementry);
			}			
			


		}while (p != 0);

		//if program gets here, no space large enough was found in the section for small allocations. 
		//set p to root and check the section for large allocations.
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
			next->recPattern = recP;

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

	printf("Testing program. Program will terminate after running your test. Please enter the number corresponding to what you would like to test:\n");
	printf("\n1 - Single char* mymalloc with myfree\n");
	printf("2 - Multiple char* mymalloc with myfree\n");
	printf("3 - Single char* mycalloc with myfree\n");
	printf("4 - Multiple char* with mycalloc, mymalloc and myfree\n");
	printf("5 - Single char* with mymalloc and myrealloc\n");
	printf("6 - Multiple char* with mymalloc, myrealloc and mycalloc\n");
	printf("\n");
	printf("Error testing Options:\n");
	printf("7 - Allocating too large of a size\n"); 
	printf("8 - Attempt to free null variable\n");
	printf("9 - Attempt to free something unable to be freed, e.g. bad pointer arithmetic\n");
	printf("10 - Attempt to free something that has already been freed\n");

	int input;
	scanf("%d", &input);

	char* cat;
	char* dog;
	char* fish;

	switch(input){

		case 1: //malloc one char*
			cat = (char*)mymalloc(10);
			strcpy(cat, "Meow");
			printf("cat has been allocated and holds the string '%s'.\n", cat);
			myfree(cat);
			printf("cat has been freed.\n");
			break;

		case 2:
			cat = (char*)mymalloc(10);
			strcpy(cat, "Meow");
			dog = (char*)mymalloc(10);
			strcpy(dog, "Woof");
			fish = (char*)mymalloc(10);
			strcpy(fish, "Blub blub");	
			printf("Cat, dog, and fish have been allocated. Cat holds string '%s', dog holds string '%s', and fish holds string '%s'.\n", cat, dog, fish);
			myfree(cat); myfree(dog); myfree(fish);
			printf("Cat, dog, and fish have been freed.\n");
			break;

		case 3:
			fish = (char*)mycalloc(7,2);
			strcpy(fish, "Blub blub");
			printf("Fish has been allocated with calloc and holds string '%s'.\n", fish);
			myfree(fish);
			printf("Fish has been freed.\n");
			break;
		
		case 4:
			cat = (char*)mymalloc(8);
			strcpy(cat, "Meow");
			dog = (char*)mycalloc(7, 3);
			strcpy(dog, "Woof");
			fish = (char*)mymalloc(4);
			strcpy(fish, "Blub blub");

			printf("Cat, dog, and fish have been allocated. Cat holds string '%s', dog holds string '%s', and fish holds string '%s'.\n", cat, dog, fish);
			myfree(cat); myfree(dog); myfree(fish);
			printf("Cat, dog, and fish have been freed.\n");
			break;

		case 5:
			dog = (char*)mymalloc(9);
			strcpy(dog, "Woof");
			printf("dog after calling mymalloc is '%s'.\n", dog);
			fish = (char*)myrealloc(dog, 11);
			strcpy(fish, "New woof");
			printf("fish after calling myrealloc with dog is '%s'.\n", fish);
			break;

		case 6:
			dog = (char*)mymalloc(10);
			strcpy(dog, "Woof");
			cat = (char*)mymalloc(8);
			strcpy(cat, "Meow");
			dog = (char*)myrealloc(dog, 12);
			strcpy(dog, "Hello");
			fish = (char*)mycalloc(4, 5);
			strcpy(fish, "Oh wow! A talking dog!");	

			printf("Dog, cat, and fish have been allocated with mymalloc, myrealloc, and mycalloc.\n");
			printf("Cat holds string '%s', dog holds string '%s', and fish holds string '%s'.\n", cat, dog, fish);
			break;
		
		case 7:
			printf("Attempting to allocate char* of size 3126567...\n");
			dog = (char*)mymalloc(3126567);
			break;

		case 8:
			fish = NULL;
			printf("Attempting to free NULL variable...\n");
			myfree(fish);
		
		case 9:
			printf("Attempting to free variable + 2...\n");
			cat = (char*)mymalloc(10);
			myfree(cat + 2);

		case 10:
			printf("Attempting to free variable twice...\n");
			cat = (char*)mymalloc(5);
			strcpy(cat, "Meow");
			myfree(cat);
			myfree(cat);
		default:
			printf("Input not recognized. Terminating program.\n");
			break;
	}
}

