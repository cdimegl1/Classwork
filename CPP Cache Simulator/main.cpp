#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "cache.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	string flag;
	unsigned long long address;

	vector<Cache*> caches;
	caches.push_back(new DirectMappedCache(1, 32));
	caches.push_back(new DirectMappedCache(4, 32));
	caches.push_back(new DirectMappedCache(16, 32));
	caches.push_back(new DirectMappedCache(32, 32));
	caches.push_back(new SetAssociativeCache(16, 32, 2));
	caches.push_back(new SetAssociativeCache(16, 32, 4));
	caches.push_back(new SetAssociativeCache(16, 32, 8));
	caches.push_back(new SetAssociativeCache(16, 32, 16));
	caches.push_back(new FullyAssociativeLRU(16, 32));
	caches.push_back(new FullyAssociativeHotCold(16, 32));
	caches.push_back(new SetAssociativeNoWriteMiss(16, 32, 2));
	caches.push_back(new SetAssociativeNoWriteMiss(16, 32, 4));
	caches.push_back(new SetAssociativeNoWriteMiss(16, 32, 8));
	caches.push_back(new SetAssociativeNoWriteMiss(16, 32, 16));
	caches.push_back(new SetAssociativePrefetch(16, 32, 2));
	caches.push_back(new SetAssociativePrefetch(16, 32, 4));
	caches.push_back(new SetAssociativePrefetch(16, 32, 8));
	caches.push_back(new SetAssociativePrefetch(16, 32, 16));
	caches.push_back(new SetAssociativePrefetchOnMiss(16, 32, 2));
	caches.push_back(new SetAssociativePrefetchOnMiss(16, 32, 4));
	caches.push_back(new SetAssociativePrefetchOnMiss(16, 32, 8));
	caches.push_back(new SetAssociativePrefetchOnMiss(16, 32, 16));

	ifstream inFile(argv[1]);
	while(inFile >> flag >> hex >> address) {
		for(Cache* cache: caches) {
			cache->sim({flag == "L" ? LOAD : STORE, address});
		}
	}
	inFile.close();

	ofstream outFile(argv[2]);
	Cache* next = caches[1];
	for(unsigned int i = 0; i < caches.size(); i++) {
		outFile << caches[i]->hits << "," << caches[i]->accesses << ";";
		if(i == caches.size() - 1) {
			outFile << "\n";
			break;
		}
		if(typeid(*next) == typeid(*caches[i])) {
			outFile << " ";
		} else {
			outFile << "\n";
		}
		next = caches[i + 2];
	}
	outFile.close();

	for(Cache* cache: caches) {
		delete cache;
	}
}
