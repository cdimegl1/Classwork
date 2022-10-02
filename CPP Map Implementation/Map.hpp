#include <utility>
#include <iostream>
#include <random>
#include <cstring>
#include <time.h>

#define SL_MAXLVL 32

namespace cs440 {
	template <typename Key_T, typename Mapped_T>
	class Map {
		public:
			class Iterator;
			class ConstIterator;
			class ReverseIterator;
			typedef std::pair<const Key_T, Mapped_T> ValueType;
			Map() : list(SkipList(.5)) {}
			Map(const Map& m) : list(m.list) {}
			Map(std::initializer_list<ValueType> l) : Map() {
				for(const ValueType& p : l) {
					insert(p);
				}
			}
			Map& operator=(const Map& m) {
				if(this != &m) {
					clear();
					new (this) Map(m);
				} else {
					Map m1(m);
					clear();
					new (this) Map(m1);
				}
				return *this;
			}
			void remove(Key_T key) {
				list.deleteElement(key);
			}
			size_t size() const {
				return list.getSize();
			}
			bool empty() const {
				return list.getSize() == 0;
			}
			Iterator begin() {
				return Iterator(list, size() == 0 ? list.tail : list.head->forward[0]);
			}
			Iterator end() {
				return Iterator(list, list.tail);
			}
			ConstIterator begin() const {
				return Iterator(list, size() == 0 ? list.tail : list.head->forward[0]);
			}
			ConstIterator end() const {
				return Iterator(list, list.tail);
			}
			ReverseIterator rbegin() {
				return Iterator(list, size() == 0 ? list.head : list.tail->reverse[0]);
			}
			ReverseIterator rend() {
				return Iterator(list, list.head);
			}
			Iterator find(const Key_T& key) {
				Node* n = 0;
				if((n = list.searchElement(key)) == nullptr) {
					return end();
				} else {
					return Iterator(list, n);
				}
			}
			ConstIterator find(const Key_T& key) const {
				return const_cast<Map*>(this)->find(key); 
			}
			Mapped_T& at(const Key_T& key) {
				Node* n = 0;
				if((n = list.searchElement(key)) == nullptr) {
					throw std::out_of_range("key not found");
				} else {
					return static_cast<DataNode*>(n)->key.second;
				}
			}
			const Mapped_T& at(const Key_T& key) const {
				return const_cast<Map*>(this)->at(key); 
			}
			Mapped_T& operator[](const Key_T& key) {
				Node* n = 0;
				if((n = list.searchElement(key)) == nullptr) {
					return insert(ValueType(key, Mapped_T())).first->second;
				} else {
					return static_cast<DataNode*>(n)->key.second;
				}
			}
			std::pair<Iterator, bool> insert(const ValueType& key) {
				return list.insertElement(key);
			}
			template <typename IT_T>
			void insert(IT_T range_beg, IT_T range_end) {
				while(range_beg != range_end) {
					insert(*range_beg);
					range_beg++;
				}
			}
			void erase(Iterator pos) {
				if(!list.deleteElement(pos->first)) {
					throw std::out_of_range("key not found");
				}
			}
			void erase(const Key_T& key) {
				if(!list.deleteElement(key)) {
					throw std::out_of_range("key not found");
				}
			}
			void clear() {
				for(Node* current = list.head; current != nullptr;) {
					Node* tmp = current->forward[0];
					delete current;
					current = tmp;
				}
				delete list.tail;
				Map();
			}
			friend bool operator==(const Map& m1, const Map& m2) {
				if(m1.size() != m2.size()) {
					return false;
				}
				Iterator it1 = const_cast<Map*>(&m1)->begin();
				Iterator it2 = const_cast<Map*>(&m2)->begin();
				for(; it1 != const_cast<Map*>(&m1)->end(); it1++, it2++) {
					if(*it1 != *it2) {
						return false;
					}
				}
				return true;
			}
			friend bool operator!=(const Map& m1, const Map& m2) {
				return !(m1 == m2);
			}
			friend bool operator<(const Map& m1, const Map& m2) {
				Iterator it1 = m1.begin();
				Iterator it2 = m2.begin();
				for(; it1 != m1.end(); it1++, it2++) {
					if(*it1 < *it2) {
						return true;
					} else if(*it1 == *it2) {
						continue;
					} else {
						return false;
					}
				}
				if(m1.size() < m2.size()) {
					return true;
				}
				return false;;
			}
			//void print() {
			//	list.print();
			//}
		private:
			class Node {
				public:
					Node** forward;
					Node** reverse;
					int levels;
					Node(int level) : forward(new Node* [level + 1]), reverse(new Node* [level + 1]), levels(level) {
						memset(forward, 0, sizeof(Node*) * (level + 1));
						memset(reverse, 0, sizeof(Node*) * (level + 1));
					}
					~Node() {
						delete forward;
						delete reverse;
					}
			};
			class DataNode : public Node {
				public:
					ValueType key;
					DataNode(const ValueType& key, int level) : Node(level), key(key) {}
			};
			class SkipList {
				public:
					const float p;
					int level;
					Node* head;
					Node* tail;
					size_t size;
					SkipList(float p) : p(p), level(0), head(new Node(SL_MAXLVL)), tail(new Node(SL_MAXLVL)), size(0) {}
					SkipList(const SkipList& sl) : SkipList(sl.p) {
						Node* current = sl.tail->reverse[0];
						for(size_t i = 0; i < sl.size; i++) {
							insertElement(static_cast<DataNode*>(current)->key);
							current = current->reverse[0];
						}
					}
					int randomLevel() {
						int lvl = 0;
						while((float)rand()/RAND_MAX < p && lvl < SL_MAXLVL) {
							lvl++;
						}
						return std::min(lvl , SL_MAXLVL);
					}
					Node* createNode(const ValueType& key, int level) {
						Node* n = new DataNode(key, level);
						return n;
					}
					std::pair<Iterator, bool> insertElement(const ValueType& key) {
						Node* current = head;
						Node* update[SL_MAXLVL + 1];
						memset(update, 0, sizeof(Node*) * (SL_MAXLVL + 1));
						for(int i = level; i >= 0; i--) {
							while(current->forward[i] != nullptr && static_cast<DataNode*>(current->forward[i])->key.first < key.first) {
								current = current->forward[i];
							}
							update[i] = current;
						}
						current = current->forward[0];
						std::pair<Iterator, bool> retVal(Iterator(*this, nullptr), true);
						if(current == nullptr || !(static_cast<DataNode*>(current)->key.first == key.first)) {
							int rlevel = randomLevel();
							if(rlevel > level) {
								for(int i = level + 1; i < rlevel + 1; i++) {
									update[i] = head;
								}
								level = rlevel;
							}
							Node* n = createNode(key, rlevel);
							retVal.first = Iterator(*this, n);
							for(int i = 0; i <= rlevel; i++) {
								n->forward[i] = update[i]->forward[i];
								Node* tmp = update[i]->forward[i];
								update[i]->forward[i] = n;
								if(tmp != nullptr) {
									tmp->reverse[i] = n;
								} else {
									tail->reverse[i] = n;
								}
								if(update[i] != head) {
									n->reverse[i] = update[i];
								}
							}
						size++;
						} else {
							return std::pair<Iterator, bool>(Iterator(*this, current), false);
						}
						return retVal;
					}
					bool deleteElement(const Key_T& key) {
						Node* current = head;
						Node* update[SL_MAXLVL + 1];
						memset(update, 0, sizeof(Node*) * (SL_MAXLVL + 1));
						for(int i = level; i >= 0; i--) {
							while(current->forward[i] != nullptr && static_cast<DataNode*>(current->forward[i])->key.first < key) {
								current = current->forward[i];
							} 
							update[i] = current;
						}
						current = current->forward[0];
						if(current != nullptr && static_cast<DataNode*>(current)->key.first == key) {
							for(int i = 0; i <= level; i++) {
								if(update[i]->forward[i] != current){
									break;
								}
								update[i]->forward[i] = current->forward[i];
								if(current->reverse[i] == nullptr) {
									if(current->forward[i] == nullptr) {
										tail->reverse[i] = nullptr;
									} else {
										current->forward[i]->reverse[i] = nullptr;
									}
								} else {
									if(current->forward[i] != nullptr) {
										current->forward[i]->reverse[i] = current->reverse[i];
									} else {
										tail->reverse[i] = current->reverse[i];
									}
								}
							}
							delete current;
							while(level > 0 && head->forward[level] == nullptr) {
								level--;
							}
							size--;
							return true;
						}
						return false;
					}
					Node* searchElement(const Key_T& key) {
						Node* current = head;
						for(int i = level; i >= 0; i--) {
							while(current->forward[i] && static_cast<DataNode*>(current->forward[i])->key.first < key) {
								current = current->forward[i];
							}
						}
						current = current->forward[0];
						if(current && static_cast<DataNode*>(current)->key.first == key) {
							return current;
						}
						return nullptr;
					}

					size_t getSize() const {
						return size;
					}
					~SkipList() {
						for(Node* current = head; current != nullptr;) {
							Node* tmp = current->forward[0];
							delete current;
							current = tmp;
						}
						delete tail;
					}
					//void print() {
					//	std::cout << "forward:\n";
					//	Node* front = head->forward[0];
					//	while(front != nullptr) {
					//		std::cout << static_cast<DataNode*>(front)->key.first << "\n";
					//		front = front->forward[0];
					//	}
					//	std::cout << "reverse:\n";
					//	front = tail->reverse[0];
					//	while(front != nullptr) {
					//		std::cout << static_cast<DataNode*>(front)->key.first << "\n";
					//		front = front->reverse[0];
					//	}
					//}
			};
			SkipList list;
			public:
				class Iterator {
					protected:
						SkipList* list;
						Node* current;
					public:
						typedef Map<Key_T, Mapped_T>::Iterator iterator;
						Iterator() = delete;
						Iterator(const SkipList& list, const Node* current) : list(const_cast<SkipList*>(&list)), current(const_cast<Node*>(current)) {}
						Iterator& operator++() {
							Node* next = current->forward[0];
							if(next == nullptr) {
								current = list->tail;
							} else {
								current = next;
							}
							return *this;
						}
						Iterator operator++(int i) {
							Iterator pre_inc(*this);
							Node* next = current->forward[0];
							if(next == nullptr) {
								current = list->tail;
							} else {
								current = next;
							}
							return pre_inc;
						}
						Iterator& operator--() {
							Node* prev = current->reverse[0];
							if(prev == nullptr) {
								current = list->head;
							} else {
								current = prev;
							}
							return *this;
						}
						Iterator operator--(int i) {
							Iterator pre_dec(*this);
							Node* prev = current->reverse[0];
							if(prev == nullptr) {
								current = list->head;
							} else {
								current = prev;
							}
							return pre_dec;
						}
						ValueType& operator*() const {
							return static_cast<DataNode*>(current)->key;
						}
						ValueType* operator->() const {
							return &static_cast<DataNode*>(current)->key;
						}
						friend bool operator!=(const Iterator& it1, const Iterator& it2) {
							return it1.current != it2.current;
						}
				};
				class ReverseIterator : public Iterator {
					public:
						ReverseIterator(const Iterator& it) : Iterator(it) {}
						ReverseIterator& operator++() {
							Iterator::operator--();
							return *this;
						}
						ReverseIterator operator++(int i) {
							return Iterator::operator--(i);
						}
						ReverseIterator& operator--() {
							Iterator::operator++();
							return *this;
						}
						ReverseIterator operator--(int i) {
							return Iterator::operator++(i);
						}
				};
				class ConstIterator : public Iterator {
					public:
						ConstIterator(const Iterator& it) : Iterator(it) {}
						ConstIterator& operator++() {
							Iterator::operator++();
							return *this;
						}
						ConstIterator operator++(int i) {
							return Iterator::operator++(i);
						}
						ConstIterator& operator--() {
							Iterator::operator--();
							return *this;
						}
						ConstIterator operator--(int i) {
							return Iterator::operator--(i);
						}
						const ValueType& operator*() const {
							return static_cast<DataNode*>(Iterator::current)->key;
						}
						const ValueType* operator->() const {
							return &static_cast<DataNode*>(Iterator::current)->key;
						}
				};
	};
}
						template<template <class, class> class M, class K, class V, class = class std::enable_if<std::is_same<M<K,V>, typename M<K,V>::iterator>::value>::type>
						bool operator==(M<K,V>& it1, M<K,V>& it2) {
							return *it1 == *it2;
						}

