# ==== Project configuration ====
TARGET      := main
SRC_DIR     := src
INC_DIR     := include
BUILD_DIR   := build
OBJ_DIR     := $(BUILD_DIR)/obj
BIN_DIR     := $(BUILD_DIR)/bin

# ==== Compiler and flags ====
CXX      := g++
IMGUI_DIR := imgui-1.92.9b
CXXFLAGS := -std=c++17 -I$(INC_DIR) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
LDFLAGS  :=

# Extended warnings, always on: help catch bugs at compile time
# (variable shadowing, bad printf/scanf formats, null dereferences, etc.)
CXXFLAGS += -Wall -Wextra -Wpedantic -Wshadow \
            -Wformat=2 -Wnull-dereference -Wdouble-promotion -Wcast-align \
            -Wundef

# Libraries via pkg-config (add/remove as needed)
PKGS     := sdl3 sdl3-image sdl3-ttf sdl3-mixer
CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LDFLAGS  += $(shell pkg-config --libs $(PKGS))

# Debug mode (use: make DEBUG=1)
ifdef DEBUG
    CXXFLAGS += -g -O0 -DDEBUG -fno-omit-frame-pointer
else
    CXXFLAGS += -O2
endif

# Sanitize mode (use: make SANITIZE=1, best combined with DEBUG=1)
# Catches at runtime: out-of-bounds access, use-after-free, memory leaks,
# undefined behavior (overflow, division by zero, invalid shifts, etc.)
ifdef SANITIZE
    CXXFLAGS += -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS  += -fsanitize=address,undefined
endif

# ==== Source/object files ====
VPATH := $(SRC_DIR):$(IMGUI_DIR):$(IMGUI_DIR)/backends
SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(IMGUI_DIR)/imgui.cpp \
        $(IMGUI_DIR)/imgui_demo.cpp \
        $(IMGUI_DIR)/imgui_draw.cpp \
        $(IMGUI_DIR)/imgui_tables.cpp \
        $(IMGUI_DIR)/imgui_widgets.cpp \
        $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp \
        $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer3.cpp

OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))
DEPS := $(OBJS:.o=.d)

# ==== Main targets ====
.PHONY: all clean run rebuild check

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo "==> Executable built: $@"

# Compile each .cpp into .o, also generating dependency files (.d)
# so changed headers trigger automatic recompilation.
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

# ==== Build directories ====
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ==== Utility targets ====
run: all
	./$(BIN_DIR)/$(TARGET)

rebuild:
	$(MAKE) clean
	$(MAKE) all

# Static code analysis
check:
	cppcheck --enable=all --inconclusive --std=c++17 -I$(INC_DIR) $(SRC_DIR)

clean:
	rm -rf $(BUILD_DIR)
	@echo "==> Clean complete"
