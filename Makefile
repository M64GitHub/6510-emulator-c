BINARY=main_cpu_test

CXX=gcc

CFLAGS+=

all: $(BINARY)

$(BINARY): $(BINARY).o \
	6510.o
	$(CXX) -o $@ $^ $(CFLAGS)

clean:
	rm $(BINARY) *.o
