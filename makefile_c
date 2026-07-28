# ==== Project configuration ====
TARGET      := main
SRC_DIR     := src
INC_DIR     := include
BUILD_DIR   := build
OBJ_DIR     := $(BUILD_DIR)/obj
BIN_DIR     := $(BUILD_DIR)/bin

# ==== Compiler and flags ====
CC      := gcc
CFLAGS  := -std=c17 -I$(INC_DIR)
LDFLAGS :=

# Extended warnings, always on: help catch bugs at compile time
# (variable shadowing, bad printf/scanf formats, null dereferences, etc.)
CFLAGS  += -Wall -Wextra -Wpedantic -Wshadow \
           -Wformat=2 -Wnull-dereference -Wdouble-promotion -Wcast-align \
           -Wstrict-prototypes -Wundef

# Libraries via pkg-config (add/remove as needed)
PKGS    := sdl3 sdl3-image sdl3-ttf sdl3-mixer
CFLAGS  += $(shell pkg-config --cflags $(PKGS))
LDFLAGS += $(shell pkg-config --libs $(PKGS))

# Debug mode (use: make DEBUG=1)
ifdef DEBUG
    CFLAGS += -g -O0 -DDEBUG -fno-omit-frame-pointer
else
    CFLAGS += -O2
endif

# Sanitize mode (use: make SANITIZE=1, best combined with DEBUG=1)
# Catches at runtime: out-of-bounds access, use-after-free, memory leaks,
# undefined behavior (overflow, division by zero, invalid shifts, etc.)
ifdef SANITIZE
    CFLAGS  += -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS += -fsanitize=address,undefined
endif

# ==== Source/object files ====
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# ==== Main targets ====
.PHONY: all clean run rebuild check

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "==> Executable built: $@"

# Compile each .c into .o, also generating dependency files (.d)
# so changed headers trigger automatic recompilation.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

# ==== Build directories ====
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ==== Utility targets ====
run: all
	./$(BIN_DIR)/$(TARGET)

rebuild: clean all

# Static code analysis
check:
	cppcheck --enable=all --inconclusive --std=c11 -I$(INC_DIR) $(SRC_DIR)

clean:
	rm -rf $(BUILD_DIR)
	@echo "==> Clean complete"
