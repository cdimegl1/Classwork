#include "my_mem_allocator.h"

extern ALLOC_POLICY alloc_policy;
extern BLOCK_HDR * fl_head;

void *my_malloc(size_t size)
{
    void * usr_ptr = NULL;
	BLOCK_HDR * last_free_block = fl_head;
	if(alloc_policy == AP_FIRST) {
		BLOCK_HDR * p = fl_head;
		do {
			if (p->size >= size + sizeof(BLOCK_HDR)) {
				usr_ptr = p;
				break;
			}
			last_free_block = p;
			p = p->next;
		} while(p);
	}
	if(alloc_policy == AP_BEST) {
		BLOCK_HDR * best = NULL;
		BLOCK_HDR * p = fl_head;
		BLOCK_HDR * prev_block = NULL;
		do {
			if (p->size > size) {
				if(best == NULL || p->size < best->size) {
					best = p;
					last_free_block = prev_block;
				}
			}
			prev_block = p;
			p = p->next;
		} while(p);
		usr_ptr = best;
	}
	if(usr_ptr == NULL) {
		printf("my_alloc error: no free block can accomodate size %#x.\n", size);
		return NULL;
	}
	BLOCK_HDR * fl_entry = (BLOCK_HDR *)((char *)usr_ptr + size + sizeof(BLOCK_HDR));
	BLOCK_HDR * block_usr_ptr = usr_ptr;
	if(block_usr_ptr->size <= size + sizeof(BLOCK_HDR)) {
		last_free_block->next = block_usr_ptr->next;
		block_usr_ptr->next = (void *)MAGIC_NUM;
	} else {
		fl_entry->size = block_usr_ptr->size - size - sizeof(BLOCK_HDR);
		fl_entry->next = block_usr_ptr->next;
		block_usr_ptr->size = size;
		block_usr_ptr->next = (void *)MAGIC_NUM;
		if(fl_head == block_usr_ptr) {
			fl_head = fl_entry;
		} else {
			if(last_free_block == fl_head) {
				fl_head->next = fl_entry;
			} else {
				last_free_block->next = fl_entry;
			}
		}
		if(fl_entry->next == (void *)fl_entry) {
			fl_entry->next == NULL;
		}
	}
	usr_ptr += sizeof(BLOCK_HDR);
	
    return usr_ptr;
}
