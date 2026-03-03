# This makefile builds and runs tests. Each test is an executable stored in the
# `./build/` directory, built from a C file placed here.

# Assume the library has been built in the expected directory
LIB       := ../lib/libjrc.a
TESTS     := $(patsubst %.c,build/%,$(wildcard *))
CFLAGS    := -g -Wall -Wextra -Wpedantic

.PHONY: all clean
all: $(TESTS)

# This is the rule for building an individual test.
build/%: %.c $(LIB)
	$(CC) $(CFLAGS) -o $@ $< $(LIB)
	./$@

clean:
	$(RM) build/*
