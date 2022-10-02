TARGET = predictors
CC = g++
CFLAGS = -O3 -pedantic -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code

$(TARGET): main.o predictor.o
	$(CC) $(CFLAGS) main.o predictor.o -o $@

main.o:	main.cpp predictor.hpp
	$(CC) $(CFLAGS) -c main.cpp
				
predictor.o: predictor.cpp predictor.hpp
	$(CC) $(CFLAGS) -c predictor.cpp

clean:
	rm -rf *.o $(TARGET)
							
