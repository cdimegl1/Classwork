#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "errors.h"
#include "knn_ocr.h"
#include "helpers.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
	if(argc < 3 || argc > 4) {
		panic("command must be invoked with 2 or 3 arguments\nusage: LD_LIBRARY_PATH=$HOME/cs551/lib ./knnc SERVER_DIR DATA_DIR [N_TESTS]");
	}
	struct stat s;
	if (stat(argv[1], &s) < 0) {
		panic("SERVER_DIR does not exist");
    }
	unsigned int n_tests = argc == 4 ? atol(argv[3]) : 10000;
	chdir(argv[1]);
	unsigned int pid = getpid();
	unsigned char pid_bytes[4];
	int requests = open("REQUESTS", O_WRONLY, 0666);
	bytes_from_uint(pid, pid_bytes); 
	if(write(requests, pid_bytes, 4) < 0) {
		panic("client write error: %s", strerror(errno));
	}
	close(requests);
	struct LabeledDataListKnn* test_data = read_labeled_data_knn(argv[2], TEST_DATA, TEST_LABELS);
	char file_name[11];
	sprintf(file_name, "%u", pid);
	mkfifo(file_name, 0666);
	int out_fd = open(file_name, O_WRONLY, 0666);
	if(out_fd < 0) {
		panic("client open error: %s", strerror(errno));
	}
	unsigned char num_tests[4];
	bytes_from_uint(n_tests, num_tests);
	if(write(out_fd, num_tests, 4) < 0) {
		panic("client write error: %s", strerror(errno));
	}
	char outfile_name[15];
	strcpy(outfile_name, file_name);
	strcat(outfile_name, "_out");
	int num_correct = 0;
	int fd = open(outfile_name, O_RDONLY, 0666);
	for(int i = 0; i < n_tests; i++) {
		struct LabeledDataKnn* element = labeled_data_at_index_knn(test_data, i);
		unsigned char* bytes = data_bytes_knn(labeled_data_data_knn(element)).bytes;
		if(write(out_fd, bytes, 784) < 0) {
			panic("client write error: %s", strerror(errno));
		}
		unsigned int index = uint_from_fd(fd);
		unsigned char result;
		if(read(fd, &result, 1) < 0) {
			panic("client read error: %s", strerror(errno));
		}
		if(result == labeled_data_label_knn(element)) {
			num_correct++;
		} else {
			printf("%u[%u] %u[%u]\n", result, index, labeled_data_label_knn(element), i);
		}
	}
	printf("%.1f%% success\n", ((float)num_correct / n_tests) * 100);
	close(fd);
	close(out_fd);
	unlink(outfile_name);
	free_labeled_data_knn(test_data);
}
