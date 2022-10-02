CC = gcc
COURSE = cs220
CPPFLAGS = -g -I $$HOME/$(COURSE)/include
LDFLAGS = -L $$HOME/$(COURSE)/lib -l cs220 -l y86

OBJ_FILES = \
  ysim.o \
  main.o

TARGET = 		y86-sim

all:			$(TARGET)

$(TARGET):	main.o ysim.o
			$(CC) $(LDFLAGS) $(OBJ_FILES) -o $(TARGET)

main.o::	main.c ysim.h
			$(CC) $(CPPFLAGS)  -c $< -o $@

ysim.o::	ysim.c ysim.h
			$(CC) $(CPPFLAGS)  -c $< -o $@

.PHONY:			clean
clean:
			rm -f $(TARGET) *.o *~
