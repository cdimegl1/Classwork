#include "my_mem_allocator.h"
#include <unistd.h>

extern BLOCK_HDR * fl_head;

void my_free(void *ptr)
{    
	if(*((int *)(ptr - sizeof(void *))) != MAGIC_NUM) {
		printf("free error: double free or corruption!\n");
		return;
	}
	*((int*)(ptr - sizeof(void *))) = 0; // remove magic number
	
	BLOCK_HDR * alloc_block = (void *)((int *)(ptr - 2 * sizeof(void *)));
	BLOCK_HDR * p = fl_head;
	BLOCK_HDR * before = NULL;
	BLOCK_HDR * after = NULL;
	BLOCK_HDR * last_free_block = NULL;
	BLOCK_HDR * first_free_block = NULL;
	int set = 0;
	do {
		if((char *)p == (char *)alloc_block + alloc_block->size + sizeof(BLOCK_HDR)) {
			after = (BLOCK_HDR *)((char *)alloc_block + alloc_block->size + sizeof(BLOCK_HDR));
		} else if ((char *)p + p->size + sizeof(BLOCK_HDR) == (char *)alloc_block) {
			before = p;
		}
		if((unsigned long)p < (unsigned long)alloc_block)
			last_free_block = p;
		if(!set && (unsigned long)p > (unsigned long)alloc_block) {
			first_free_block = p;
			set = 1;
		}
		p = p->next;
	} while(p);
	
	if(before && after) {
		before->size += alloc_block->size + after->size + 2 * sizeof(BLOCK_HDR);
		before->next = after->next;
	} else if(before && !after) {
		before->size += alloc_block->size + sizeof(BLOCK_HDR);
	} else if(!before && after) {
		alloc_block->size += after->size + sizeof(BLOCK_HDR);
		alloc_block->next = after->next;
		if(last_free_block) {
			last_free_block->next = alloc_block;
		} else {
			fl_head = alloc_block;
		}
	} else {
		if(first_free_block != fl_head) {
			alloc_block->next = last_free_block->next;
			last_free_block->next = alloc_block;
		} else {
			alloc_block->next = fl_head;
			fl_head = alloc_block;
		}
	}
    return;
}
