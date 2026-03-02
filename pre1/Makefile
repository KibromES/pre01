# Authors: Odin Bjerke <odin.bjerke@uit.no>
# Modified by: Morten Grønnesby <morten.gronnesby@uit.no>

# Commands
CC ?= gcc
DEBUG ?= 1
EXE = wordfreq

# Directories
SRC_DIR = src
INCLUDE = include
OBJ_DIR = obj
BIN_DIR = bin

RELEASE_DIR = $(BIN_DIR)/release
DEBUG_DIR = $(BIN_DIR)/debug

# Source and object files
SRC := $(filter-out $(SRC_DIR)/benchmark.c, $(wildcard $(SRC_DIR)/*.c))
HEADERS := $(wildcard $(INCLUDE)/*.h)
OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# linked libraries
LDFLAGS += -lm

# specify c/libc standard
CFLAGS += -std=c2x -D_GNU_SOURCE

# options for printing.h. LOG_LEVEL may be set per-file, or globally, like here.
# CFLAGS += -D LOG_LEVEL=LOG_LEVEL_WARN
# CFLAGS += -D PRINTING_NCOLOR
# CFLAGS += -D PRINTING_NMETA

# Turn off debugprints and utilize highest optimization level
ifeq ($(DEBUG), 0)
CFLAGS += -O3 -DNDEBUG
BUILD_DIR := $(RELEASE_DIR)
TARGET := $(BUILD_DIR)/$(EXE)
else
CFLAGS += -Og -DDEBUG -g -Wall -Wextra -Wno-constant-logical-operand
BUILD_DIR := $(DEBUG_DIR)
TARGET := $(BUILD_DIR)/$(EXE)
endif


.PHONY: all exec
.PHONY: clean distclean
.PHONY: dirs

all: dirs exec
exec: $(TARGET)

$(TARGET): $(OBJ) $(HEADERS) Makefile
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $< -o $@

dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BUILD_DIR)

clean:
	rm -f $(OBJ)
	rm -rf $(RELEASE_DIR)
	rm -rf $(DEBUG_DIR)


# Benchmark target
BENCH_SRC := $(filter-out $(SRC_DIR)/main.c $(SRC_DIR)/testmap.c $(SRC_DIR)/testlist.c $(SRC_DIR)/testutil.c $(SRC_DIR)/benchmark.c, $(wildcard $(SRC_DIR)/*.c)) $(SRC_DIR)/benchmark.c
BENCH_OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/bench_%.o,$(BENCH_SRC))

bench: dirs $(BUILD_DIR)/benchmark

$(BUILD_DIR)/benchmark: $(BENCH_OBJ) $(HEADERS)
	$(CC) $(BENCH_OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/bench_%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $< -o $@

distclean: clean
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
