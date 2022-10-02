TARGETS = minimal test1 test2 performance
all: $(TARGETS)
$(TARGETS): Map.hpp
	g++ -O3 -o $@ $@.cpp
clean:
	rm $(TARGETS)
