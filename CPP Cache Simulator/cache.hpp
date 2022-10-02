#include <vector>
#include <cmath>
#include <list>

enum LS_FLAG {
	LOAD,
	STORE
};

struct Instruction {
	LS_FLAG flag;
	unsigned long long address;
};

class Cache {
	public:
		unsigned long long hits;
		unsigned long long accesses;
		virtual bool sim(Instruction i) = 0;
		Cache() : hits(0), accesses(0) {}
		virtual ~Cache() {}
};

struct Address {
	unsigned int index;
	unsigned int tag;
	Address(unsigned long long address, unsigned int cache_B, unsigned int lineSize) {
		address >>= (unsigned int)log2(lineSize);
		index = address & ((cache_B / lineSize) - 1);
		address >>= (unsigned int)log2(cache_B) / lineSize;
		tag = 0xffffffffffffffff & address;
	}
	Address() : index(0), tag(0) {}
};

struct Entry {
	Address addr;
	bool isValid;
	Entry() : addr(), isValid(false) {}
};

class DirectMappedCache : public Cache {
	private:
		unsigned int cache_B;
		unsigned int lineSize;
		size_t maxSize;
		Entry* cache;
	public:
		DirectMappedCache(unsigned int cache_KB, unsigned int lineSize) : cache_B(cache_KB * 1024), lineSize(lineSize), maxSize((1024 * cache_KB) / lineSize), cache(new Entry[maxSize]) {}
		bool sim(Instruction i) {
			accesses++;
			Address a(i.address, cache_B, lineSize);
			Entry* e = &cache[a.index];
			if(e->isValid && a.tag == e->addr.tag) {
				hits++;
				return true;
			} else {
				e->isValid = true;
				e->addr.tag = a.tag;
			}
			return false;
		}
		~DirectMappedCache() {
			delete [] cache;
		}
};

class SetAssociativeCache : public Cache {
	protected:
		unsigned cache_B;
		unsigned int lineSize;
		unsigned int associativity;
		unsigned int numSets;
		Entry** sets;
		std::list<unsigned int>* lru;
		void lru_push(std::list<unsigned int>& lru, unsigned int i) {
			lru.remove(i);
			lru.push_back(i);
		}
	public:
		SetAssociativeCache() = delete;
		SetAssociativeCache(unsigned int cache_KB, unsigned int lineSize, unsigned int associativity) : cache_B(cache_KB * 1024), lineSize(lineSize), associativity(associativity), numSets(cache_B / lineSize / associativity), lru(new std::list<unsigned int>[numSets]) {
			sets = new Entry*[numSets];
			for(unsigned int i = 0; i < numSets; i++) {
				sets[i] = new Entry[associativity];
			}
		}
		bool sim(Instruction i) {
			accesses++;
			Address a(i.address, cache_B / associativity, lineSize);
			for(unsigned int j = 0; j < associativity; j++) {
				Entry* e = &sets[a.index][j];
				if(e->isValid) {
					if(a.tag == e->addr.tag) {
						hits++;
						lru_push(lru[a.index], j);
						return true;
					} else {
						continue;
					}
				} else {
					e->isValid = true;
					e->addr.tag = a.tag;
					lru_push(lru[a.index], j);
					return false;
				}
			}
			unsigned int lru_val = lru[a.index].front();
			lru[a.index].pop_front();
			lru[a.index].push_back(lru_val);
			Entry* e = &sets[a.index][lru_val];
			e->addr.tag = a.tag;
			return false;
		}
		~SetAssociativeCache() {
			for(unsigned int i = 0; i < numSets; i++) {
				delete [] sets[i];
			}
			delete [] sets;
			delete [] lru;
		}
};

class FullyAssociativeLRU : public Cache {
	private:
		SetAssociativeCache cache;
	public:
		FullyAssociativeLRU() = delete;
		FullyAssociativeLRU(unsigned int cache_KB, unsigned int lineSize) : cache(cache_KB, lineSize, cache_KB * 1024 / lineSize) {}
		bool sim(Instruction i) {
			accesses++;
			bool retVal = cache.sim(i);
			if(retVal) hits++;
			return retVal;
		}
};

struct Node {
	Entry e;
	bool cold;
	Node() : e(), cold(0) {}
};

inline unsigned int left(unsigned int i) {
	return (2 * i) + 1;
}

inline unsigned int right(unsigned int i) {
	return (2 * i) + 2;
}

class FullyAssociativeHotCold : public Cache {
	private:
		unsigned int cache_B;
		unsigned int lineSize;
		unsigned int leaves;
		unsigned int levels;
		Node* tree;
	public:
		FullyAssociativeHotCold(unsigned int cache_KB, unsigned int lineSize) : cache_B(cache_KB * 1024), lineSize(lineSize), leaves(cache_B / lineSize), levels(log2(leaves)), tree(new Node[(cache_B / lineSize * 2) - 1]) {}
		bool sim(Instruction i) {
			accesses++;
			Address a(i.address, 0, lineSize);
			unsigned int index;
			for(index = leaves - 1; index < (leaves * 2) - 1; index++) {
				if(tree[index].e.isValid) {
					if(tree[index].e.addr.tag == a.tag) {
						unsigned int curr = index;
						for(unsigned int k = 0; k < levels; k++) {
							if(curr % 2 == 0) {
								curr = (curr - 2) / 2;
								tree[curr].cold = 1;
							} else {
								curr = (curr - 1) / 2;
								tree[curr].cold = 0;
							}
						}
						hits++;
						return true;
					} else {
						continue;
					}
				}
			}
			unsigned int leaf_index = 0;
			for(unsigned int j = 0; j < levels; j++) {
				if(tree[leaf_index].cold) {
					tree[leaf_index].cold = 0;
					leaf_index = left(leaf_index);
				} else {
					tree[leaf_index].cold = 1;
					leaf_index = right(leaf_index);
				}
			}
			Node* n = &tree[leaf_index];
			n->e.addr.tag = a.tag;
			n->e.isValid = true;
			return false;
		}
		~FullyAssociativeHotCold() {
			delete [] tree;
		}
};

class SetAssociativeNoWriteMiss : public SetAssociativeCache {
	public:
		SetAssociativeNoWriteMiss(unsigned int cache_KB, unsigned int lineSize, unsigned int associativity) : SetAssociativeCache(cache_KB, lineSize, associativity) {}
		bool sim(Instruction i) {
			accesses++;
			Address a(i.address, cache_B / associativity, lineSize);
			for(unsigned int j = 0; j < associativity; j++) {
				Entry* e = &sets[a.index][j];
				if(e->isValid) {
					if(a.tag == e->addr.tag) {
						hits++;
						lru_push(lru[a.index], j);
						return true;
					} else {
						continue;
					}
				} else {
					if(i.flag == LOAD) {
						e->isValid = true;
						e->addr.tag = a.tag;
						lru_push(lru[a.index], j);
						return false;
					}
				}
			}
			if(i.flag == LOAD) {
				unsigned int lru_val = lru[a.index].front();
				lru[a.index].pop_front();
				lru[a.index].push_back(lru_val);
				Entry* e = &sets[a.index][lru_val];
				e->addr.tag = a.tag;
			}
			return false;
		}
};

class SetAssociativePrefetch : public SetAssociativeCache {
	private:
		void fetchNext(Instruction i) {
			Address a(i.address + lineSize, cache_B / associativity, lineSize);
			unsigned int j;
			for(j = 0; j < associativity; j++) {
				Entry* e = &sets[a.index][j];
				if(e->isValid) {
					if(a.tag == e->addr.tag) {
						lru_push(lru[a.index], j);
						return;
					} else {
						continue;
					}
				} else {
					e->isValid = true;
					e->addr.tag = a.tag;
					lru_push(lru[a.index], j);
					return;
				}
			}
			unsigned int lru_val = lru[a.index].front();
			lru[a.index].pop_front();
			lru[a.index].push_back(lru_val);
			Entry* e = &sets[a.index][lru_val];
			e->addr.tag = a.tag;
			return;
		}
	public:
		SetAssociativePrefetch(unsigned int cache_KB, unsigned int lineSize, unsigned int associativity) : SetAssociativeCache(cache_KB, lineSize, associativity) {}
		bool sim(Instruction i) {
			accesses++;
			Address a(i.address, cache_B / associativity, lineSize);
			unsigned int j;
			for(j = 0; j < associativity; j++) {
				Entry* e = &sets[a.index][j];
				if(e->isValid) {
					if(a.tag == e->addr.tag) {
						hits++;
						lru_push(lru[a.index], j);
						fetchNext(i);
						return true;
					} else {
						continue;
					}
				} else {
					e->isValid = true;
					e->addr.tag = a.tag;
					lru_push(lru[a.index], j);
					fetchNext(i);
					return false;
				}
			}
			unsigned int lru_val = lru[a.index].front();
			lru[a.index].pop_front();
			lru[a.index].push_back(lru_val);
			Entry* e = &sets[a.index][lru_val];
			e->addr.tag = a.tag;
			fetchNext(i);
			return false;
		}
};

class SetAssociativePrefetchOnMiss : public SetAssociativeCache {
	private:
		void fetchNext(Instruction i) {
			Address a(i.address + lineSize, cache_B / associativity, lineSize);
			unsigned int j;
			for(j = 0; j < associativity; j++) {
				Entry* e = &sets[a.index][j];
				if(e->isValid) {
					if(a.tag == e->addr.tag) {
						lru_push(lru[a.index], j);
						return;
					} else {
						continue;
					}
				} else {
					e->isValid = true;
					e->addr.tag = a.tag;
					lru_push(lru[a.index], j);
					return;
				}
			}
			unsigned int lru_val = lru[a.index].front();
			lru[a.index].pop_front();
			lru[a.index].push_back(lru_val);
			Entry* e = &sets[a.index][lru_val];
			e->addr.tag = a.tag;
			return;
		}
	public:
		SetAssociativePrefetchOnMiss(unsigned int cache_KB, unsigned int lineSize, unsigned int associativity) : SetAssociativeCache(cache_KB, lineSize, associativity) {}
		bool sim(Instruction i) {
			accesses++;
			Address a(i.address, cache_B / associativity, lineSize);
			unsigned int j;
			for(j = 0; j < associativity; j++) {
				Entry* e = &sets[a.index][j];
				if(e->isValid) {
					if(a.tag == e->addr.tag) {
						hits++;
						lru_push(lru[a.index], j);
						return true;
					} else {
						continue;
					}
				} else {
					e->isValid = true;
					e->addr.tag = a.tag;
					lru_push(lru[a.index], j);
					fetchNext(i);
					return false;
				}
			}
			unsigned int lru_val = lru[a.index].front();
			lru[a.index].pop_front();
			lru[a.index].push_back(lru_val);
			Entry* e = &sets[a.index][lru_val];
			e->addr.tag = a.tag;
			fetchNext(i);
			return false;
		}
};

