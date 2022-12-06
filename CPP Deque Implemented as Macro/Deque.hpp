#ifndef _DEQUE_hpp
#define _DEQUE_hpp

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const size_t DEQUE_STARTING_CAPACITY = 10;

#define Deque_DEFINE(t) \
	typedef struct Deque_##t##_Iterator {\
		t* array;\
		bool hasMoved;\
		size_t pos;\
		size_t capacity;\
		void (*dec)(Deque_##t##_Iterator* it);\
		void (*inc)(Deque_##t##_Iterator* it);\
		t& (*deref)(Deque_##t##_Iterator* it);\
	} Deque_##t##_Iterator; \
\
	typedef struct Deque_##t {\
		char type_name[sizeof("Deque_"#t)];\
		bool (*empty)(Deque_##t*);\
		void (*clear)(Deque_##t*);\
		int _front;\
		int rear;\
		size_t _size;\
		size_t capacity;\
		bool (*compare)(const t&, const t&);\
		Deque_##t##_Iterator (*begin)(Deque_##t*);\
		Deque_##t##_Iterator (*end)(Deque_##t*);\
		t& (*at)(Deque_##t* d, unsigned int i);	\
		void (*dtor)(Deque_##t*);\
		size_t (*size)(Deque_##t*);\
		void (*push_back)(Deque_##t*, t);		\
		void (*push_front)(Deque_##t*, t);\
		void(*pop_back)(Deque_##t*);\
		void (*pop_front)(Deque_##t*);\
		t& (*back)(Deque_##t*);\
		t& (*front)(Deque_##t*);\
		void (*sort)(Deque_##t*, Deque_##t##_Iterator, Deque_##t##_Iterator);	\
		t* array;\
	} Deque_##t;\
\
	bool Deque_##t##_empty(Deque_##t* d) {\
		return !d->_size;\
	}\
\
	void Deque_##t##_clear(Deque_##t* d) {\
		d->_front = -1;\
		d->rear = 0;\
		d->_size = 0;\
	}\
\
	void Deque_##t##_dec(Deque_##t##_Iterator* it) {\
		it->pos = it->pos != 0 ? it->pos - 1 : it->capacity - 1;\
		it->hasMoved = true;\
	}\
\
	void Deque_##t##_inc(Deque_##t##_Iterator* it) {\
		it->pos = (it->pos + 1) % it->capacity;\
		it->hasMoved = true;\
	}\
\
	t& Deque_##t##_deref(Deque_##t##_Iterator* it) {\
		return it->array[it->pos];\
	}\
\
	t& Deque_##t##_at(Deque_##t* d, unsigned int i) {\
		return d->array[(d->_front + i) % d->capacity];	\
	}\
\
	void Deque_##t##_dtor(Deque_##t* d) {\
		free(d->array);\
	}\
\
	bool Deque_##t##_Iterator_equal(Deque_##t##_Iterator begin, Deque_##t##_Iterator end) {\
		return begin.pos == end.pos && begin.hasMoved;\
	} \
\
	size_t Deque_##t##_size(Deque_##t* d) {\
		return d->_size;\
	}\
\
	void Deque_##t##_pop_back(Deque_##t* d) {\
		if(d->_size == 0) return;\
		if(d->_front == d->rear) {\
			d->_front = -1;\
			d->rear = -1;\
		} else if(d->rear == 0) {\
			d->rear = d->capacity - 1;\
		} else {\
			d->rear -= 1;\
		}\
		d->_size -= 1;\
	}\
\
	void Deque_##t##_pop_front(Deque_##t* d) {\
		if(d->_size == 0) return;\
		if(d->_front == d->rear) {\
			d->_front = -1;\
			d->rear = -1;\
		} else if((size_t)d->_front == d->capacity - 1) {\
			d->_front = 0;\
		} else {\
			d->_front += 1;\
		}\
		d->_size -= 1;\
	}\
\
	t& Deque_##t##_back(Deque_##t* d) {\
		return d->array[d->rear];\
	}\
\
	t& Deque_##t##_front(Deque_##t* d) {\
		return d->array[d->_front];\
	}\
\
	int t##_comparator(const void* first, const void* second, void* deque) {\
		t* f = (t*)first;\
		t* s = (t*)second;\
		Deque_##t* d = (Deque_##t*)deque;\
		if(d->compare(*f, *s)) {\
			return -1;\
		}\
		if(d->compare(*s, *f)) {\
			return 1;\
		}\
		return 0;\
	}\
\
	Deque_##t##_Iterator Deque_##t##_begin(Deque_##t* d) {\
		return Deque_##t##_Iterator {d->array, false, (size_t)d->_front, d->capacity, &Deque_##t##_dec, &Deque_##t##_inc, &Deque_##t##_deref};\
	}\
\
	Deque_##t##_Iterator Deque_##t##_end(Deque_##t* d) {\
		return {d->array, false, (d->rear + 1) % d->capacity, d->capacity, &Deque_##t##_dec, &Deque_##t##_inc, &Deque_##t##_deref};\
	}\
\
	void __realloc(Deque_##t* d) {\
		t* new_array = (t*)malloc(sizeof(t) * d->capacity * 2);\
		int i = 0;\
		for(Deque_##t##_Iterator it = Deque_##t##_begin(d); !Deque_##t##_Iterator_equal(it, Deque_##t##_end(d)); Deque_##t##_inc(&it), i++) {\
			new_array[i] = Deque_##t##_deref(&it);\
		}\
		free(d->array);\
		d->array = new_array;\
		d->_front = 0;\
		d->rear = d->_size - 1;\
		d->capacity *= 2;\
	}\
\
	void Deque_##t##_push_back(Deque_##t* d, t i) {\
		if(d->_front == -1) {\
			d->_front = 0;\
			d->rear = 0;\
		} else if((size_t)d->rear == d->capacity - 1) {\
			d->rear = 0;\
		} else {\
			d->rear += 1;\
		}\
		d->array[d->rear] = i;\
		d->_size += 1;\
		if(d->_size == d->capacity) {\
			__realloc(d);\
		}\
	}\
\
	void Deque_##t##_push_front(Deque_##t* d, t i) {\
		if(d->_front == -1) {\
			d->_front = 0;\
			d->rear = 0;\
		} else if(d->_front == 0) {\
			d->_front = d->capacity - 1;\
		} else {\
			d->_front -= 1;\
		}\
		d->array[d->_front] = i;\
		d->_size += 1;\
		if(d->_size == d->capacity) {\
			__realloc(d);\
		}\
	}\
\
	bool Deque_##t##_equal(Deque_##t& d1, Deque_##t& d2) { \
		if(d1._size != d2._size) {return false;}                                  \
		Deque_##t##_Iterator it1 = Deque_##t##_begin(&d1);  \
		Deque_##t##_Iterator it2 = Deque_##t##_begin(&d2);  \
		while(!Deque_##t##_Iterator_equal(it1, Deque_##t##_end(&d1))) { \
			if(d1.compare(Deque_##t##_deref(&it1), Deque_##t##_deref(&it2)) != d2.compare(Deque_##t##_deref(&it2), Deque_##t##_deref(&it1))) {return false;}                              \
			Deque_##t##_inc(&it1);                                     \
			Deque_##t##_inc(&it2);                                     \
		}                                                  \
		return true;                                       \
	}                                                      \
\
	void Deque_##t##_sort(Deque_##t* d, Deque_##t##_Iterator begin, Deque_##t##_Iterator end) {\
		int start_pos = begin.pos;\
		if(begin.pos > end.pos) {\
			int seg1_start = begin.pos;\
			Deque_##t##_dec(&end);\
			int seg2_end = end.pos; \
			Deque_##t##_inc(&end);\
			qsort_r(&d->array[seg1_start], d->capacity - seg1_start, sizeof(t), t##_comparator, d);\
			qsort_r(d->array, seg2_end + 1, sizeof(t), t##_comparator, d);\
		} else {\
			size_t dist = 0;\
			for(; !Deque_##t##_Iterator_equal(begin, end); Deque_##t##_inc(&begin), dist++);\
			qsort_r(&d->array[start_pos], dist, sizeof(t), t##_comparator, d);\
		}		\
	}\
\
	void Deque_##t##_ctor(Deque_##t* d, bool (*f)(const t&, const t&)) {\
		strcpy(d->type_name, "Deque_"#t);\
		d->empty = &Deque_##t##_empty;\
		d->clear = &Deque_##t##_clear;\
		d->_front = -1;\
		d->rear = 0;\
		d->_size = 0;\
		d->capacity = DEQUE_STARTING_CAPACITY;\
		d->compare = f;\
		d->array = (t*)malloc(sizeof(t) * DEQUE_STARTING_CAPACITY);\
		d->begin = &Deque_##t##_begin;\
		d->end = &Deque_##t##_end;\
		d->at = &Deque_##t##_at;\
		d->dtor = &Deque_##t##_dtor;\
		d->size = &Deque_##t##_size;\
		d->push_back = &Deque_##t##_push_back;\
		d->push_front = &Deque_##t##_push_front;\
		d->pop_front = &Deque_##t##_pop_front;\
		d->pop_back = &Deque_##t##_pop_back;\
		d->back = &Deque_##t##_back;\
		d->front = &Deque_##t##_front;\
		d->sort = &Deque_##t##_sort;	\
		d->dtor = &Deque_##t##_dtor;\
	}

#endif
