#ifndef _SHAREDPTR_HPP
#define _SHAREDPTR_HPP
#include <mutex>

namespace cs540 {
	std::mutex ref_lock;
	class GenericDtor {
		public:
			virtual void deleter() = 0;
			virtual ~GenericDtor() {}
	};
	template <typename T>
	class Dtor : public GenericDtor {
		public:
			T* ptr;
			Dtor() : ptr(0) {}
			Dtor(T* p) : ptr(p) {}
			void deleter() {
				delete ptr;
			}
	};
	struct Counter {
		GenericDtor* dtor;
		int refs;
		Counter() : dtor(0), refs(0) {}
		template <typename T>
		Counter(T* p) : dtor(new Dtor<T>(p)), refs(1) {}
		~Counter() {
			if(dtor) {
				dtor->deleter();
				delete dtor;
			}
		}
	};
	template <typename T>
	class SharedPtr {
		private:
			template <typename U>
			friend class SharedPtr;
			Counter* counter;
			T* ptr;
			void dec() {
				ref_lock.lock();
				if(counter) {
					if(--counter->refs == 0) {
						delete counter;
						counter = 0;
						ptr = 0;
					}
				}
				ref_lock.unlock();
			}
			void inc() {
				if(counter) {
					++counter->refs;
				}
				ref_lock.unlock();
			}
		public:
			template <typename U>
			SharedPtr(const SharedPtr<U> &p, T* t) : counter(p.counter), ptr(t){
				inc();
			}
			Counter* get_counter() const {
				return counter;
			}
			SharedPtr(Counter* c, T* p) : counter(c), ptr(p) {}
			SharedPtr() : counter(0), ptr(0) {}
			template <typename U>
			explicit SharedPtr(U* p) : counter(new Counter(p)), ptr(p) {}
			SharedPtr(const SharedPtr& p) : counter(p.counter), ptr(p.ptr) {
				inc();
			}
			template <typename U>
			SharedPtr(const SharedPtr<U>& p) : counter(p.counter), ptr(static_cast<T*>(p.ptr)) {
				inc();
			}
			SharedPtr(SharedPtr&& p) : counter(p.counter), ptr(p.ptr) {
				p.counter = 0;
				p.ptr = 0;
			}
			template <typename U>
			SharedPtr(SharedPtr<U>&& p) : counter(p.counter), ptr(p.ptr) {
				p.counter = 0;
				p.ptr = 0;
			}
			SharedPtr& operator=(const SharedPtr &p) {
				if(this == &p) {
					return *this;
				}
				dec();
				counter = p.counter;
				ptr = p.ptr;
				inc();
				return *this;

			}
			template <typename U>
			SharedPtr<T>& operator=(const SharedPtr &p) {
				if(this == (void*)&p) {
					return *this;
				}
				dec();
				counter = p.counter;
				ptr = p.ptr;
				inc();
				return *this;
			}
			SharedPtr &operator=(SharedPtr &&p){
				if(this == &p){
					return *this;
				}
				dec();
				counter = p.counter;
				ptr = p.ptr;
				p.counter = 0;
				p.ptr = 0;
				return *this;
			}
			template <typename U>
			SharedPtr<T> &operator=(SharedPtr<U> &&p){
				if(this == &p){
					return *this;
				}
				dec();
				counter = p.counter;
				ptr = p.ptr;
				p.counter = 0;
				p.ptr = 0;
				return *this;
			}
			~SharedPtr() {
				dec();
			}
			void reset() {
				dec();
				counter = 0;
				ptr = 0;
			}
			template <typename U>
			void reset(U* p) {
				dec();
				counter = new Counter(p);
				ptr = p;
			}
			T* get() const {
				return ptr;
			}
			T& operator*() const {
				return *ptr;
			}
			T* operator->() const {
				return ptr;	
			}
			explicit operator bool() const {
				return ptr != 0;
			}
	};
	template <typename T1, typename T2>
	bool operator==(const SharedPtr<T1>& p1, const SharedPtr<T2>& p2) {
		return p1.get() == p2.get();
	}
	template <typename T>
	bool operator==(const SharedPtr<T>& p, std::nullptr_t np) {
		return p.get() == np;
	}
	template <typename T>
	bool operator==(std::nullptr_t np, const SharedPtr<T>& p) {
		return p.get() == np;
	}
	template <typename T1, typename T2>
	bool operator!=(const SharedPtr<T1>& p1, const SharedPtr<T2>& p2) {
		return p1.get() != p2.get();
	}
	template <typename T>
	bool operator!=(const SharedPtr<T>& p, std::nullptr_t np) {
		return p.get() != np;
	}
	template <typename T>
	bool operator!=(std::nullptr_t np, const SharedPtr<T>& p) {
		return p.get() != np;
	}
	template <typename T, typename U>
	SharedPtr<T> static_pointer_cast(const SharedPtr<U> &p) {
		return SharedPtr<T>(p, static_cast<T*>(p.get()));
	}
	template <typename T, typename U>
	SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &p) {
		return SharedPtr<T>(p, dynamic_cast<T*>(p.get()));
	}
}
#endif

