TARGET = test
CC = g++
CFLAGS = -pedantic -Wall -Wextra -g

$(TARGET): test.o
	$(CC) $(CFLAGS) -o $@ test.o -lpthread
	
test.o:	 test.cpp SharedPtr.hpp
	$(CC) $(CFLAGS) -c test.cpp

clean:
	rm -rf *.o $(TARGET)
