CC = gcc
CFLAGS = -Wall -DBSD -O3

all: knnd knnc

knnd: knnd.o
	$(CC) $(CFLAGS) -o knnd knnd.o -L../../../cs551/lib -lcs551 -lknn_ocr
	
knnc: knnc.o
	$(CC) $(CFLAGS) -o knnc knnc.o -L../../../cs551/lib -lcs551 -lknn_ocr

knnd.o:	knnd.c
	$(CC) $(CFLAGS) -c knnd.c -I../../../cs551/include

knnc.o: knnc.c
	$(CC) $(CFLAGS) -c knnc.c -I../../../cs551/include

depend:
	gcc -MM knnd.c -I../../../cs551/include

clean:
	rm -rf *.o knnd knnc

