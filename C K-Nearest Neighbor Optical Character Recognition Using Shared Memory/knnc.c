#include "errors.h"
#include "memalloc.h"
#include "knn_ocr.h"
#include "sem_shm_defs.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>

typedef struct {
	const char* posixName;
	int oflags;
} SemOpenArgs;

static SemOpenArgs semArgs[] = {
	{ .posixName = SERVER_SEM_NAME,
	  .oflags = O_RDWR,
	},
	{ .posixName = REQUEST_SEM_NAME,
	  .oflags = O_RDWR,
	},
	{ .posixName = RESPONSE_SEM_NAME,
	  .oflags = O_RDWR,
	},
};

int main(int argc, char* argv[]) {
	if(argc < 2 || argc > 3) {
		panic("command must be invoked with 1 or 2 arguments\nusage: LD_LIBRARY_PATH=$HOME/cs551/lib ./knnc DATA_DIR [N_TESTS]");
	}
	int n_tests = argc == 3 ? atoi(argv[2]) : 10000;

	sem_t* sems[N_SEMS];
	for(int i = 0; i < N_SEMS; i++) {
		const SemOpenArgs* p = &semArgs[i];
		if((sems[i] = sem_open(p->posixName, p->oflags)) == NULL) {
			panic("cannot create semaphore: %s", p->posixName);
		}
	}
	int shm = shm_open(SHM_NAME, O_RDWR, 0);
	if(shm < 0) panic("cannot create shm: %s", SHM_NAME);
	unsigned int* shared_mem = NULL;
	if((shared_mem = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED) {
		panic("cannot mmap shm %s", SHM_NAME);
	}
	const struct LabeledDataListKnn* test_data = read_labeled_data_knn(argv[1], TEST_DATA, TEST_LABELS);
	semWait(sems[SERVER_SEM], SERVER_SEM_NAME);
	*shared_mem = n_tests;
	semPost(sems[REQUEST_SEM], REQUEST_SEM_NAME);
	semWait(sems[RESPONSE_SEM], RESPONSE_SEM_NAME);
	int num_correct = 0;
	for(int i = 0; i < n_tests; i++) {
		const struct LabeledDataKnn* labeled_data = labeled_data_at_index_knn(test_data, i);
		unsigned char* bytes = data_bytes_knn(labeled_data_data_knn(labeled_data)).bytes;
		unsigned char* image_bytes = (unsigned char*)shared_mem;
		for(int j = 0; j < 784; j++) {
			*image_bytes = *bytes;
			image_bytes++;
			bytes++;
		}
		semPost(sems[REQUEST_SEM], REQUEST_SEM_NAME);
		semWait(sems[RESPONSE_SEM], RESPONSE_SEM_NAME);
		unsigned int index = *shared_mem;
		unsigned int result = *(shared_mem + 1);
		if(result == labeled_data_label_knn(labeled_data)) {
			num_correct++;
		} else {
			printf("%u[%u] %u[%u]\n", result, index, labeled_data_label_knn(labeled_data), i);
		}
	}
	printf("%.1f%% success\n", ((float)num_correct / n_tests) * 100);
	semPost(sems[SERVER_SEM], SERVER_SEM_NAME);
	for(int i = 0; i < N_SEMS; i++) {
		if(sem_close(sems[i]) < 0) {
			panic("cannot close semaphore: %s", semArgs[i].posixName);
		}
	}
	free_labeled_data_knn((struct LabeledDataListKnn*)test_data);
}
