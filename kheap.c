#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

bool first_time = 1;




#define HEAP_PAGES ((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE)
int HeapTrack[HEAP_PAGES] = {0};


void* kmalloc(unsigned int size)
{

    static int call_count = 0;
    call_count++;

    if (first_time)
    {
        HeapTrack[0] = -1 * HEAP_PAGES;
        HeapTrack[HEAP_PAGES - 1] = -1 * HEAP_PAGES;
        first_time = 0;

    }

    if(size==0)
    {
      return NULL;
    }
    int Used_size = size;
    while (Used_size % PAGE_SIZE != 0)
    {
        Used_size++;
    }
    int num_of_pages = Used_size / PAGE_SIZE;



    int maxIndex = -1;
    int maxSize = -1;
    int i=0;
    while(i<HEAP_PAGES){
        if (HeapTrack[i] > 0)
        {
            i += HeapTrack[i];  // skip allocated block
        } else if (HeapTrack[i] < 0)
        {
            int currentSize = -1 * HeapTrack[i];
            if (currentSize >= num_of_pages && currentSize > maxSize)
            {
                maxSize = currentSize;
                maxIndex = i;
            }
            i += currentSize;//skip unallocated block
        }
        else
        {

            i+=-1 * HeapTrack[i];//for extra checking :) ,might never use it tho :( i'm afraid to remove it
        }
    }

    // No sufficient block found
    if (maxSize < num_of_pages || maxIndex == -1) {

        return NULL;
    }


    HeapTrack[maxIndex] = num_of_pages;
    HeapTrack[maxIndex + num_of_pages - 1] = num_of_pages;

    // Split the remaining part if bigger than
    if (maxSize > num_of_pages) {
        int rem = maxSize - num_of_pages;
        HeapTrack[maxIndex + num_of_pages] = -1 * rem;
        HeapTrack[maxIndex + maxSize - 1] = -1 * rem;
    }

    // mapping
    for (int i = 0; i < num_of_pages; i++) {
        uint32* ptr_table = NULL;
        uint32 va = ((maxIndex + i) * PAGE_SIZE) + KERNEL_HEAP_START;
        struct Frame_Info* ptr_frame_info;


        int ret = allocate_frame(&ptr_frame_info);
        if (ret == E_NO_MEM) {

            return NULL;
        }

        ret = map_frame(ptr_page_directory, ptr_frame_info, (void*)va,PERM_WRITEABLE);
        if (ret == E_NO_MEM) {


            return NULL;
        }
        else{
        	frames_info[to_physical_address(ptr_frame_info)/PAGE_SIZE].va= va;
        }
    }


    return (void*)(KERNEL_HEAP_START + maxIndex * PAGE_SIZE);
}

void kfree(void* virtual_address)
{
    int index = ((uint32)virtual_address - KERNEL_HEAP_START) / PAGE_SIZE;
	int Base_page=HeapTrack[index];
	if(Base_page<0){
		return;
	}
	int total=Base_page;
	int first=index;
	int last=index+Base_page-1;
	int before=-1;
	int after=-1;
	if(Base_page>0){
	before=index-1;
	after=index+Base_page;
		}
	HeapTrack[first]=0;
	HeapTrack[last]=0;


	if(before >= 0 && HeapTrack[before] < 0){
	total+=(-HeapTrack[before]);
	first = before - (-HeapTrack[before]) + 1;
	}
	if(after < HEAP_PAGES && HeapTrack[after] < 0)
	{
		total+=(-HeapTrack[after]);
		last = after + (-HeapTrack[after]) - 1;
	}
	HeapTrack[first]=-total;
	HeapTrack[last]=-total;
	 for (int i = 0; i < Base_page; i++)
	 {
		    uint32* ptr_table = NULL;
		    unsigned char *va = (unsigned char *)(KERNEL_HEAP_START + (index + i) * PAGE_SIZE);


		    struct Frame_Info* ptr_frame_info =get_frame_info(ptr_page_directory,(void*)va,&ptr_table);
		    cprintf("yelo");
		    free_frame(ptr_frame_info);
		    cprintf("hey");
	        unmap_frame(ptr_page_directory, va);
	        frames_info[to_physical_address(ptr_frame_info)/PAGE_SIZE].va=0;


	   }




}
unsigned int kheap_virtual_address(unsigned int physical_address)
{
           //o(1)

		unsigned int frame_idx = to_frame_number(to_frame_info(physical_address));



		if (frame_idx >=number_of_frames) {
			//Invalid physical address
			return 0;
		}
		if (frames_info[frame_idx].va == 0) {
				//Frame not allocated
				return 0;
			}
		if(frames_info[frame_idx].va< KERNEL_HEAP_START || frames_info[frame_idx].va> KERNEL_HEAP_MAX)
		{
		//out of bounds
			return 0;
		}

        unsigned int va=frames_info[frame_idx].va;
		return va+(physical_address%PAGE_SIZE);
		//o(n)
		/* uint32* ptr_page_table = NULL;
		    struct Frame_Info* target_frame = to_frame_info(physical_address);

		    if (target_frame == NULL)
		    {
		        // Physical address isn't a valid frame
		        return 0;
		    }

		    uint32 va = KERNEL_HEAP_START;
		    while (va != KERNEL_HEAP_MAX)
		    {
		        struct Frame_Info* current_frame = get_frame_info(ptr_page_directory, (void*)va, &ptr_page_table);

		        if (current_frame == target_frame)
		            return va+(physical_address%PAGE_SIZE);


		        va += PAGE_SIZE;
		    }

		    if(va<KERNEL_HEAP_START||va>KERNEL_HEAP_MAX)
		    return 0;*/


}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2025 - MS1 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer'

	uint32* ptr_table = NULL;
	struct Frame_Info* ptr_frame_info =get_frame_info(ptr_page_directory,(void*)virtual_address,&ptr_table);
	if(ptr_frame_info!=NULL){
	return to_physical_address(ptr_frame_info)+(virtual_address%PAGE_SIZE);
	}
	else{

		return 0;
	}




}

void *krealloc(void *virtual_address, uint32 new_size)
{
	panic("krealloc() is not required...!!");
	return NULL;

}
