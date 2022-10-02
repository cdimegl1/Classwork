TARGET = dimeglio_a1
CC = g++
CFLAGS = -pedantic -Wall -Wextra -Werror -Wshadow -Wconversion -Wunreachable-code

$(TARGET): test.o
	$(CC) $(CFLAGS) -o $@ test.o -ldl
	
test.o:	 test.cpp Deque.hpp
	$(CC) $(CFLAGS) -c test.cpp

clean:
	rm -rf *.o $(TARGET)
