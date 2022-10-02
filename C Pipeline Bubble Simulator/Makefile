CC = gcc
COURSE = cs220
CPPFLAGS = -g -I $$HOME/$(COURSE)/include
LDFLAGS = -L $$HOME/$(COURSE)/lib -l cs220 -l y86

OBJ_FILES = \
  stall-sim.o \
  main.o

TARGET = 		stall-sim

all:			$(TARGET)

$(TARGET):	main.o stall-sim.o
			$(CC) $(LDFLAGS) $(OBJ_FILES) -o $(TARGET)

main.o::	main.c
			$(CC) $(CPPFLAGS)  -c $< -o $@

stall-sim.o::	stall-sim.c stall-sim.h
			$(CC) $(CPPFLAGS)  -c $< -o $@
			
depend:
		$(CC) -MM $(CPPFLAGS) *.c

.PHONY:			clean
clean:
			rm -f $(TARGET) *.o *~
