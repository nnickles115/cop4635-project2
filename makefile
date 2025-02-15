# Compiler
CXX = g++

# Flags - Needs C++17
CXXFLAGS = -std=gnu++17 -g -Wall

# Debug flags
DEBUG_FLAGS = -fdiagnostics-color=always -fsanitize=address

# Library files
INCLUDES = -Iinclude -Iinclude/common -Iinclude/message -Iinclude/network

# Source files
SRCS = $(shell find src -name "*.cpp")

# Object output directory
OBJ_DIR = build

# Object files
OBJS = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Target executable
TARGET = client

# Default target
all: client

# Debugging target
debug: override CXXFLAGS += $(DEBUG_FLAGS)
debug: all

# Ensure the object directory and subdirectories exist
$(OBJ_DIR):
	@if [ "$(OBJS)" ]; then \
		for dir in $(sort $(dir $(OBJS))); do \
			mkdir -p "$$dir"; \
		done; \
	fi;

# Object file pattern rule (without boost)
$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile all sources to .o files and link them to the target
client: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

# Clean up the build files
clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*/*.o $(TARGET)

# Prevent make from looking for files with these names
.PHONY: all clean debug client
