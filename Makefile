# --- Project Settings ---------------------------------------------------------
NAME         := jrc
VERSION      := 0.0.1

# ------ Configuration ---------------------------------------------------------
# Path for installation.
PREFIX       ?= /usr/local
# Compilation profile (release / debug).
PROFILE      ?= release

# ------ Paths -----------------------------------------------------------------
# Directories
SRC_DIR      := src
# We put the header along with the source files
INC_DIR      := include
OBJ_BASE_DIR := obj
LIB_BASE_DIR := lib
OBJ_DIR      := $(OBJ_BASE_DIR)/$(PROFILE)
LIB_DIR      := $(LIB_BASE_DIR)/$(PROFILE)
BIN_DIR      := $(OBJ_BASE_DIR)/$(PROFILE)/bin
INSTALL_LIB  := $(PREFIX)/lib
INSTALL_INC  := $(PREFIX)/include/$(NAME)

# ------ Files -----------------------------------------------------------------
TARGET       := $(LIB_DIR)/lib$(NAME).a
SRCS         := $(wildcard $(SRC_DIR)/*.c)
HEADERS      := $(wildcard $(INC_DIR)/*.h)
OBJS         := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- Toolchain ----------------------------------------------------------------
CC           ?= gcc
AR           := ar
ARFLAGS      := rcs
CFLAGS       := $(shell cat compile_flags.txt) -MMD -MP

# ------ Profile Specific Settings ---------------------------------------------
ifeq ($(PROFILE), debug)
	CFLAGS += -g -O0 -DDEBUG
else ifeq ($(PROFILE), release)
	CFLAGS += -O3 -DNDEBUG
else
	$(error "Invalid PROFILE: $(PROFILE). Use 'debug' or 'release'.")
endif

# --- Targets ------------------------------------------------------------------
.PHONY: build release debug test fmt clean install uninstall help repl

build: $(TARGET)

release:
	@# Set the profile to release and recursively call make
	@$(MAKE) PROFILE=release

debug:
	@# Set the profile to debug and recursively call make
	@$(MAKE) PROFILE=debug

## Build the static library
$(TARGET): $(OBJS) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^
	@echo "Built static library: $@"

## Compile each source file into an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%: CFLAGS += -DBIN
$(BIN_DIR)/%: $(SRC_DIR)/%.c $(HEADERS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

-include $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)

## Create output directories if they don't exist
$(OBJ_DIR) $(LIB_DIR) $(BIN_DIR):
	mkdir -p $@

# --- Test ---------------------------------------------------------------------
# The tests have their own separate makefile, so we just give them the .a file
# and delegate to them
test: debug test/*
	cd test && make -j 8

fmt:
	@echo "Formatting source files with clang-format"
	@# -i - in-place editing
	clang-format -i $(SRCS) $(HEADERS)

repl: $(BIN_DIR)/repl
	$(BIN_DIR)/repl 

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
	$(RM) -r $(OBJ_BASE_DIR) $(LIB_BASE_DIR)
	cd test && make clean
	@echo "Cleaned build artifacts"

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "  build     Build the static library (default)"
	@echo "  debug     Build with debug symbols and no optimisation"
	@echo "  test      Run all tests"
	@echo "  fmt       Format source files with clang-format"
	@echo "  repl      Run a REPL for testing the interpreter implemented in the library"
	@echo "  install   Install library and headers to PREFIX (default: /usr/local)"
	@echo "  uninstall Remove installed files"
	@echo "  clean     Remove all build artifacts"
	@echo "  help      Show this message"
