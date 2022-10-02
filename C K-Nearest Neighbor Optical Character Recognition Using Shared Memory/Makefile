CC = gcc
CFLAGS = -Wall -g -std=gnu11 -O3

all: knnd knnc

knnd: knnd.o
	$(CC) $(CFLAGS) -o knnd knnd.o -L../../../cs551/lib -lcs551 -lknn_ocr -lrt -lpthread

knnc: knnc.o
	$(CC) $(CFLAGS) -o knnc knnc.o -L../../../cs551/lib -lcs551 -lknn_ocr -lrt -lpthread

knnd.o: knnd.c sem_shm_defs.h
	$(CC) $(CFLAGS) -c knnd.c -I../../../cs551/include

knnc.o: knnc.c sem_shm_defs.h
	$(CC) $(CFLAGS) -c knnc.c -I../../../cs551/include

clean:
	rm -rf *.o knnd knnc

