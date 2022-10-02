TARGET = cache-sim
CC = g++
CFLAGS = -g -O3 -Wall

$(TARGET): main.o
	$(CC) $(CFLAGS) main.o -o $@

main.o: main.cpp cache.hpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o $(TARGET)
