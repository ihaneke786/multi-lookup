# multi-lookup — multithreaded producer/consumer DNS resolver

PROJECT := multi-lookup
CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -g -std=gnu99 -pthread
SRCS := multi-lookup.c array.c DNSlookup.c
HDRS := multi-lookup.h array.h util.h
OBJS := $(SRCS:.c=.o)
INPUT := input/names1.txt input/names2.txt input/names3.txt

.PHONY: all release clean distclean help run run-full bench bench-quick

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CC) -o $@ $^ -lpthread

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

release: clean
	$(MAKE) all CFLAGS='-O2 -DNDEBUG -Wall -Wextra -std=gnu99 -pthread'

clean:
	$(RM) $(OBJS) $(PROJECT)

distclean: clean
	$(RM) -f serviced*.txt results*.txt

help:
	@echo "all (default), release, clean, distclean"
	@echo "run, run-full, bench-quick, bench"
	@echo "./$(PROJECT) <requesters> <resolvers> <serviced> <results> <inputs...>"

run: $(PROJECT)
	./$(PROJECT) 2 2 serviced.txt results.txt $(INPUT)

run-full: $(PROJECT)
	./$(PROJECT) 10 10 serviced.txt results.txt input/*.txt

bench-quick: $(PROJECT)
	@echo "1×1:"; ./$(PROJECT) 1 1 /tmp/s1.txt /tmp/r1.txt $(INPUT) 2>&1 | grep 'total time'
	@echo "10×10:"; ./$(PROJECT) 10 10 /tmp/s2.txt /tmp/r2.txt $(INPUT) 2>&1 | grep 'total time'

bench: $(PROJECT)
	@echo "1×1:"; ./$(PROJECT) 1 1 /tmp/s1.txt /tmp/r1.txt input/*.txt 2>&1 | grep 'total time'
	@echo "10×10:"; ./$(PROJECT) 10 10 /tmp/s2.txt /tmp/r2.txt input/*.txt 2>&1 | grep 'total time'
