#include "mymalloc.h"



void* mymalloc(unsigned int size, const char* file, const int line){

	static struct mementry *root = NULL;
	struct mementry* p = NULL;

	static int initialized = 0; //use boolean to know if memblock has been initialized
	static struct mementry *prev, *next;


	if(!initialized){
		root = (struct mementry*) memblock;
		root->prev = 0;
		root->next = 0;
		root->size = 5000 - sizeof(struct mementry);
		root->isFree = 1;
		root->recPattern = recP;

		initialized = 1;
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


	fprintf(stderr, "ERROR: Requested memory allocation at line %d in file %s of size %d is too large.\n", line, file, size);
	_exit(0);

}

void myfree(void* p, const char* file, int line){

	if(p == NULL){
		//trying to free unallocated variable	
		fprintf(stderr, "ERROR: Requested free at line %d in file %s cannot be completed; the variable was not allocated.\n", line, file);
		_exit(0);
	
	}


	struct mementry *ptr, *prev, *next;


	printf("cat from myfree is %s\n", p);
	ptr = (struct mementry*)((char*) p - sizeof(struct mementry));


	printf("pointers size is %d\n", sizeof(p));
	printf("mem entry size is %d\n", sizeof(struct mementry));	

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
		//
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

#define mymalloc( x ) mymalloc( x, __FILE__, __LINE__)
#define myfree( x ) myfree( x, __FILE__, __LINE__)

int main(){

	char* cat;

	myfree(cat);

	//char* cat = (char*)mymalloc(2000);
	
	//strcpy(cat, "meow!");

//	char* dog = (char*)mymalloc(100); 
//	strcpy(dog, "woof");
//	printf("%s\n", dog);

}
