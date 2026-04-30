# --- Project Settings ---------------------------------------------------------
NAME         := jrc
VERSION      := 0.0.1

# Directories
SRC_DIR      := src
# We put the header along with the source files
INC_DIR      := src
OBJ_DIR      := obj
LIB_DIR      := lib
PREFIX       ?= /usr/local
INSTALL_LIB  := $(PREFIX)/lib
INSTALL_INC  := $(PREFIX)/include/$(NAME)

# Output
TARGET       := $(LIB_DIR)/lib$(NAME).a

# --- Toolchain ----------------------------------------------------------------
CC           := gcc
AR           := ar
ARFLAGS      := rcs
CFLAGS       := -Wall -Wextra -Wpedantic -std=c11 -O3 -I$(INC_DIR)
DBGFLAGS     := -g -O0 -DDEBUG

# --- Sources & Objects --------------------------------------------------------
SRCS         := $(wildcard $(SRC_DIR)/*.c)
OBJS         := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- Targets ------------------------------------------------------------------
.PHONY: build debug test clean install uninstall help

build: $(TARGET)

## Build the static library
$(TARGET): $(OBJS) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^
	@echo "Built static library: $@"

## Compile each source file into an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

## Debug build (no optimisation, with symbols)
debug: CFLAGS += $(DBGFLAGS)
debug: clean $(TARGET)

## Create output directories if they don't exist
$(OBJ_DIR) $(LIB_DIR):
	mkdir -p $@


# --- Test ---------------------------------------------------------------------
# The tests have their own separate makefile, so we just give them the .a file
# and delegate to them
test: debug test/*
	cd test && make

# --- Install & Uninstall ------------------------------------------------------
install: build
	@echo "Making sure installation directories exist"
	install -d $(INSTALL_LIB) $(INSTALL_INC)
	@echo "Copying files"
	install -m 0644 $(TARGET) $(INSTALL_LIB)/
	install -m 0644 $(INC_DIR)/*.h $(INSTALL_INC)/
	@echo "Installed to $(PREFIX)"

uninstall:
	$(RM) $(INSTALL_LIB)/lib$(NAME).a
	$(RM) -r $(INSTALL_INC)
	@echo "Uninstalled from $(PREFIX)"

# --- Utility ------------------------------------------------------------------
clean:
	$(RM) -r $(OBJ_DIR) $(LIB_DIR)
	@echo "Cleaned build artifacts"

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "  build     Build the static library (default)"
	@echo "  debug     Build with debug symbols and no optimisation"
	@echo "  test      Run all tests"
	@echo "  install   Install library and headers to PREFIX (default: /usr/local)"
	@echo "  uninstall Remove installed files"
	@echo "  clean     Remove all build artifacts"
	@echo "  help      Show this message"
