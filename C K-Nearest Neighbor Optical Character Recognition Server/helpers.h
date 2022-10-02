#include <unistd.h>
#include <errno.h>

unsigned int uint_from_fd(int fd) {
	unsigned char bytes[4];
	int res;
	while((res = read(fd, bytes, 4)) == 0) {
		if(res < 0) {
			panic("uint_from_fd read failed: %s", strerror(errno));
		}
	}
	return ((unsigned int)bytes[0] << 24) | ((unsigned int)bytes[1] << 16) | ((unsigned int)bytes[2] << 8) | (unsigned int)bytes[3];
}

void bytes_from_uint(unsigned int i, unsigned char buffer[]) {
	buffer[3] = (unsigned char)(i & 0xff);
	buffer[2] = (unsigned char)((i >> 8) & 0xff);
	buffer[1] = (unsigned char)((i >> 16) & 0xff);
	buffer[0] = (unsigned char)((i >> 24) & 0xff);
}
