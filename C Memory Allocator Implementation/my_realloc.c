#include "my_mem_allocator.h"

extern BLOCK_HDR * fl_head;

void *my_realloc(void *ptr, size_t size)
{
    void * usr_ptr = NULL;
	
	if(*((int *)(ptr - sizeof(void *))) != MAGIC_NUM) {
		printf("realloc error: invalid old ptr!\n");
		return NULL;
	}
	
	BLOCK_HDR * alloc_block = (void *)((int *)(ptr - 2 * sizeof(void *)));
	
	if(size < alloc_block->size) {
		if(alloc_block->size - size > sizeof(BLOCK_HDR)) {
			BLOCK_HDR * fl_entry = (BLOCK_HDR *)((char *)alloc_block + sizeof(BLOCK_HDR) + size);
			fl_entry->size = alloc_block->size - size - sizeof(BLOCK_HDR);
			alloc_block->size = size;
			BLOCK_HDR * p = fl_head;
			do {
				if((unsigned long)p->next > (unsigned long)fl_entry) {
					fl_entry->next = p->next;
					p->next = fl_entry;
					break;
				}
				p = p->next;
			} while(p);
			BLOCK_HDR * after_fl_entry = fl_entry->next;
			if((char *)after_fl_entry == (char *)fl_entry + sizeof(BLOCK_HDR) + fl_entry->size) {
				fl_entry->size += sizeof(BLOCK_HDR) + after_fl_entry->size;
				fl_entry->next = after_fl_entry->next;
			}
		}
		usr_ptr = ptr;
	} else {
		usr_ptr = my_malloc(size);
		char * data = ptr;
		for(int i = 0; i < size; i++, data++) {
			((char *)usr_ptr)[i] = *data;
		}
		my_free(ptr);
	}

    return usr_ptr;
}
