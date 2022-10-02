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
	const char *posixName;
	int oflags;
	mode_t mode;
	unsigned initValue;
} SemOpenArgs;

static SemOpenArgs semArgs[] = {
	{ .posixName = SERVER_SEM_NAME,
	  .oflags = O_RDWR|O_CREAT,
	  .mode = ALL_RW_PERMS,
	  .initValue = 1,
	},
	{ .posixName = REQUEST_SEM_NAME,
	  .oflags = O_RDWR|O_CREAT,
	  .mode = ALL_RW_PERMS,
	  .initValue = 0,
	},
	{ .posixName = RESPONSE_SEM_NAME,
	  .oflags = O_RDWR|O_CREAT,
	  .mode = ALL_RW_PERMS,
	  .initValue = 0,
	},
};

int main(int argc, char* argv[]) {
	if(argc < 2 || argc > 3) {
		panic("command must be invoked with 1 or 2 arguments\nusage: LD_LIBRARY_PATH=$HOME/cs551/lib ./knnd DATA_DIR [K]");
	}
	int k = argc == 3 ? atoi(argv[2]) : 3;

	sem_t* sems[N_SEMS];
	for(int i = 0; i < N_SEMS; i++) {
		const SemOpenArgs* p = &semArgs[i];
		if((sems[i] = sem_open(p->posixName, p->oflags, p->mode, p->initValue)) == NULL) {
			panic("cannot create semaphore: %s", p->posixName);
		}
	}
	int shm = shm_open(SHM_NAME, O_RDWR|O_CREAT, ALL_RW_PERMS);
	if(shm < 0) panic("cannot create shm: %s", SHM_NAME);
	if(ftruncate(shm, 1024) < 0) {
		panic("cannot size shm %s to %d", SHM_NAME, 1024);
	}
	unsigned int* shared_mem = NULL;
	if((shared_mem = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED) {
		panic("cannot mmap shm %s", SHM_NAME);
	}
	printf("memory attached at %p\n", shared_mem);
	const struct LabeledDataListKnn* train_data = read_labeled_data_knn(argv[1], TRAINING_DATA, TRAINING_LABELS);
	while(1) {
		semWait(sems[REQUEST_SEM], REQUEST_SEM_NAME);
		int n_tests = *shared_mem;
		semPost(sems[RESPONSE_SEM], RESPONSE_SEM_NAME);
		for(int i = 0; i < n_tests; i++) {
			semWait(sems[REQUEST_SEM], REQUEST_SEM_NAME);
			const struct DataBytesKnn data_bytes = { 784, (unsigned char*)shared_mem };
			unsigned int index = knn_from_data_bytes(train_data, &data_bytes, k);
			*shared_mem = index;
			*(shared_mem + 1) = labeled_data_label_knn(labeled_data_at_index_knn(train_data, index));
			semPost(sems[RESPONSE_SEM], RESPONSE_SEM_NAME);
		}
	}
}
