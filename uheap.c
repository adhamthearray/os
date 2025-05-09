
#include <inc/lib.h>

// malloc()
//	This function use BEST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
#define UHEAP_PAGES ((USER_HEAP_MAX- USER_HEAP_START) / PAGE_SIZE)
int UHeapTrack[UHEAP_PAGES] = {0};
bool first_time=1;
void* malloc(uint32 size)
{

	//TODO: [PROJECT 2025 - MS2 - [2] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//void* kmalloc(unsigned int size)

		if(size<=0)
			return NULL;
	    if (first_time) {
	        UHeapTrack[0] = -1 * UHEAP_PAGES;
	        UHeapTrack[UHEAP_PAGES - 1] = -1 * UHEAP_PAGES;
	        first_time = 0;
	    }
	    // Round size to nearest multiple of PAGE_SIZE
	    int Used_size = size;
	    while (Used_size % PAGE_SIZE != 0) {
	        Used_size++;
	    }
	    int num_of_pages = Used_size / PAGE_SIZE;
	    cprintf("num: %d\n", num_of_pages);
	    // Find first-fit free block

	    int i=0;
	    int currentSize=0;

	    while(i<UHEAP_PAGES){
	        if (UHeapTrack[i] > 0) {
	            i += UHeapTrack[i];  // skip allocated block
	        } else if (UHeapTrack[i] < 0) {
	        	   currentSize = -1 * UHeapTrack[i];
	            if (currentSize >= num_of_pages) {
	            	 // Allocate the block
	            	    UHeapTrack[i] = num_of_pages;
	            	    UHeapTrack[i + num_of_pages - 1] = num_of_pages;

	            	    // Split the remaining part if larger
	            	    if (currentSize > num_of_pages) {
	            	        int rem = currentSize - num_of_pages;
	            	        UHeapTrack[i + num_of_pages] = -1 * rem;
	            	        UHeapTrack[i + currentSize - 1] = -1 * rem;
	            	    }
	            	    sys_allocateMem(USER_HEAP_START +i* PAGE_SIZE,size);
	            	return (void*)(USER_HEAP_START +i* PAGE_SIZE);
	            }
	            i += currentSize;
	        } else {
	            i+=-1 * UHeapTrack[i];  // prevent infinite loop
	        }
	    }
//	    sys_isUHeapPlacementStrategyFIRSTFIT();
return NULL;
	}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not required...!!");

	// Steps:
	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer
	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	// Write your code here, remove the panic and write your code
	panic("sget() is not required ...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement BEST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer
	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.


void free(void* virtual_address)
{
    // Round down to page boundary
    uint32 virtual_address_new = (uint32)virtual_address & ~(PAGE_SIZE - 1);

    // Calculate index in tracking array
    int index = (virtual_address_new - USER_HEAP_START) / PAGE_SIZE;

    // Get the number of pages for this allocation
    int Base_page = UHeapTrack[index];
    cprintf("wee: %d\n", Base_page);
    if (Base_page <= 0) {
        cprintf("Can not free this page, already free page\n");
        return;
    }

    sys_freeMem(virtual_address_new, Base_page * PAGE_SIZE);
    // Store values for coalescing
    int total = Base_page;
    int first = index;
    int last = index + Base_page - 1;

    // Check before (previous block)
    int before = index - 1;
    if (before >= 0 && UHeapTrack[before] < 0) {
        // If previous block is free, coalesce
        total += (-UHeapTrack[before]);
        first = before - (-UHeapTrack[before]) + 1;
    }

    // Check after (next block)
    int after = index + Base_page;
    if (after < UHEAP_PAGES && UHeapTrack[after] < 0) {
        // If next block is free, coalesce
        total += (-UHeapTrack[after]);
        last = after + (-UHeapTrack[after]) - 1;
    }

    // Update tracking array: negative values represent free blocks
    // Set first entry to negative total size
    UHeapTrack[first] = -total;
    // Set last entry to negative total size
    UHeapTrack[last] = -total;

    // Clear entries between first and last (but not first or last)
    for (int i = first + 1; i < last; i++) {
        UHeapTrack[i] = 0;
    }

    // Free memory at system level

}




//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not required ...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	// Write your code here, remove the panic and write your code
	panic("realloc() is not required yet...!!");

}
