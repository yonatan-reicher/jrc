# This makefile builds and runs tests. Each test is an executable stored in the
# `./build/` directory, built from a C file placed here.

# Assume the library has been built in the expected directory
LIB       := ../lib/libjrc.a
TESTS     := $(patsubst %.c,build/%,$(wildcard *.c))
RUN_TESTS := $(patsubst %.c,run/%,$(wildcard *.c))
CFLAGS    := -g -Wall -Wextra -Wpedantic

.PHONY: all clean
all: $(TESTS) $(RUN_TESTS)
	@echo 'All tests successful ✓'

# This is the rule for building an individual test.
build/%: %.c $(LIB) build
	$(CC) $(CFLAGS) -o $@ $< $(LIB)

build:
	mkdir -p build

# This is the run for running a test! The 'run/' directory is just for making
# a different rule for running, it does not refer to a real directory.
run/% : | build/%
	@echo "Running $*"
	@./build/$*

clean:
	$(RM) build/*
