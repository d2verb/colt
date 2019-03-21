BIN=colt

SRCS := $(shell find . -type f -name "*.c")
OBJS := $(addsuffix .o,$(basename $(SRCS)))

all: $(OBJS)
	gcc -o $(BIN) $^

.cpp.o:
	gcc -c $<

clean:
	rm -f $(OBJS) $(BIN)

.PHONY: clean
