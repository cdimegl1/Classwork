#include "errors.h"
#include <errno.h>
#include "file-io.h"
#include "knn_ocr.h"
#include "memalloc.h"
#include "trace.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "helpers.h"

void do_worker(char* file_name, const struct LabeledDataListKnn* train_data, int k) {
	mkfifo(file_name, 0666);
	char outfile_name[15];
	strcpy(outfile_name, file_name);
	strcat(outfile_name, "_out");
	mkfifo(outfile_name, 0666);
	int fd = open(file_name, O_RDONLY, 0666);
	struct DataBytesKnn to_classify = {784, (unsigned char*)mallocChk(784)};
	unsigned int num_tests = uint_from_fd(fd);
	int out_fd = open(outfile_name, O_WRONLY, 0666);
	for(int i = 0; i < num_tests; i++) {
		if(read(fd, to_classify.bytes, 784) < 0) {
			panic("daemon worker read failed: %s", strerror(errno));
		}
		unsigned char knn_result[5];
		unsigned int index = knn_from_data_bytes(train_data, &to_classify, k);
		bytes_from_uint(index, knn_result);
		knn_result[4] = labeled_data_label_knn(labeled_data_at_index_knn(train_data, index));
		if(write(out_fd, knn_result, 5) < 0) {
			panic("daemon worker write failed: %s", strerror(errno));
		}
	}
	close(fd);
	close(out_fd);
	unlink(file_name);
	free(to_classify.bytes);
}

void create_worker(char* file_name, const struct LabeledDataListKnn* train_data, int k) {
	pid_t pid = fork();
	if(pid < 0) {
		panic("worker fork error");
	} else if(pid == 0) {
		if((pid = fork()) < 0) {
			panic("worker fork error");
		} else if(pid > 0) {
			exit(0);
		}
		do_worker(file_name, train_data, k);
		exit(0);
	}
	if(waitpid(pid, NULL, 0) != pid) {
		panic("waitpid error");
	}
}

void do_server(char* data_dir, int k) {
	const struct LabeledDataListKnn* train_data = read_labeled_data_knn(data_dir, TRAINING_DATA, TRAINING_LABELS);
	mkfifo("REQUESTS", 0666);
	int fd = open("REQUESTS", O_RDONLY);
	while(1) {
		unsigned int pid = uint_from_fd(fd);
		char file_name[11];
		sprintf(file_name, "%u", pid);
		create_worker(file_name, train_data, k);
	};
}

void daemon_init(char* server_dir, char* data_dir, int k) {
	pid_t pid;
	if((pid = fork()) < 0) {
		panic("fork for daemon failed");
	} else if (pid != 0) {
		printf("daemon PID: %d\n", pid);
		exit(0);
	} else {
		mkdir(server_dir, 0777);
		setsid();
		chdir(server_dir);
		umask(0);
		do_server(data_dir, k);
	}
}

int main(int argc, char* argv[]) {
	if(argc < 3 || argc > 4) {
		panic("command must be invoked with 2 or 3 arguments\nusage: LD_LIBRARY_PATH=$HOME/cs551/lib ./knnd SERVER_DIR DATA_DIR [K]");
	}
	int k = argc == 4 ? atol(argv[3]) : 3;
	daemon_init(argv[1], argv[2], k);
	return 0;
}
