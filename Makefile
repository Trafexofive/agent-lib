# Compiler
CXX := clang++
# CXX := g++

# Target Executable Names
TARGET_BIN := agent-bin
TARGET_SERVER := agent-server

# Directories
BUILD_DIR := build
SRC_DIR := src
EXT_DIR := externals
SERVER_DIR := server
INC_DIRS := inc $(SERVER_DIR)/vendor/httplib
# --- yaml-cpp ---
# NOTE: If yaml-cpp headers are installed in a standard location searched by pkg-config,
# adding its specific path to INC_DIRS might not be strictly necessary when using pkg-config for CFLAGS.
# If pkg-config isn't used or headers are elsewhere, add the path here:
# INC_DIRS += /path/to/yaml-cpp/include

# Flags
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2 -g

# --- yaml-cpp ---
# Use pkg-config to get compiler flags (includes -I paths etc.)
# Replace 'yaml-cpp' if your pkg-config file has a different name (e.g., yaml-cpp-0.7)
# You can find the name using: pkg-config --list-all | grep -i yaml
YAML_CPP_CFLAGS := $(shell pkg-config --cflags yaml-cpp 2>/dev/null)

# Construct CPPFLAGS including standard includes and yaml-cpp includes
CPPFLAGS := $(foreach dir,$(INC_DIRS),-I$(dir)) $(YAML_CPP_CFLAGS)
# Fallback if pkg-config fails or isn't used:
# CPPFLAGS := $(foreach dir,$(INC_DIRS),-I$(dir)) -I/usr/local/include # Or specific yaml-cpp path

# --- yaml-cpp ---
# Add yaml-cpp library link flag using pkg-config
YAML_CPP_LIBS := $(shell pkg-config --libs yaml-cpp 2>/dev/null)
# Original LDFLAGS + yaml-cpp libs
LDFLAGS := -lcurl -ljsoncpp -pthread $(YAML_CPP_LIBS)
# Fallback if pkg-config fails or isn't used:
# LDFLAGS := -lcurl -ljsoncpp -pthread -lyaml-cpp -L/usr/local/lib # Add -L if library is non-standard path

# Source files - use recursive wildcard to find all source files
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MAIN_SRC := main.cpp
SERVER_SRC := $(SERVER_DIR)/server.cpp

# Recursively find all source files in src/ and externals/
SRC_SOURCES := $(call rwildcard,$(SRC_DIR),*.cpp)
EXT_SOURCES := $(call rwildcard,$(EXT_DIR),*.cpp)

COMMON_SOURCES := $(SRC_SOURCES) $(EXT_SOURCES)
ALL_SOURCES := $(MAIN_SRC) $(SERVER_SRC) $(COMMON_SOURCES)

# Object files with proper subdirectory structure
MAIN_OBJ := $(BUILD_DIR)/$(MAIN_SRC:.cpp=.o)
SERVER_OBJ := $(BUILD_DIR)/$(SERVER_SRC:.cpp=.o)

# Generate object file paths with proper directory structure
SRC_OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC_SOURCES))
EXT_OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(EXT_SOURCES))

COMMON_OBJECTS := $(SRC_OBJECTS) $(EXT_OBJECTS)
ALL_OBJECTS := $(MAIN_OBJ) $(SERVER_OBJ) $(COMMON_OBJECTS)

# Dependency files
DEPS := $(ALL_OBJECTS:.o=.d)

# Default target
all: $(TARGET_SERVER)

# Rule to build 'bin' executable
$(TARGET_BIN): $(MAIN_OBJ) $(COMMON_OBJECTS)
	@echo "Linking $(TARGET_BIN)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) # LDFLAGS now includes yaml-cpp
	@echo "$(TARGET_BIN) built successfully."

# Rule to build server executable
$(TARGET_SERVER): $(SERVER_OBJ) $(COMMON_OBJECTS)
	@echo "Linking $(TARGET_SERVER)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) # LDFLAGS now includes yaml-cpp
	@echo "$(TARGET_SERVER) built successfully."

# Generic rule for compiling any .cpp file
# CPPFLAGS now includes yaml-cpp include paths
$(BUILD_DIR)/%.o: %.cpp
	@echo "Compiling $< -> $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEPS)

# Convenience targets
bin: $(TARGET_BIN)
server: $(TARGET_SERVER)

clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET_BIN) $(TARGET_SERVER)
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Rebuild targets
re: clean all
re-bin: clean bin
re-server: clean server

# Run targets
run-bin: bin
	@echo "Running $(TARGET_BIN)..."
	./$(TARGET_BIN)

run-server: server
	@echo "Running $(TARGET_SERVER)..."
	./$(TARGET_SERVER)

# Debug target to see what source files are found
debug:
	@echo "Source files found:"
	@echo "  Main: $(MAIN_SRC)"
	@echo "  Server: $(SERVER_SRC)"
	@echo "  SRC_SOURCES: $(SRC_SOURCES)"
	@echo "  EXT_SOURCES: $(EXT_SOURCES)"
	@echo ""
	@echo "Object files to build:"
	@echo "  MAIN_OBJ: $(MAIN_OBJ)"
	@echo "  SERVER_OBJ: $(SERVER_OBJ)"
	@echo "  SRC_OBJECTS: $(SRC_OBJECTS)"
	@echo "  EXT_OBJECTS: $(EXT_OBJECTS)"
	@echo ""
	@echo "Compiler Flags (CPPFLAGS): $(CPPFLAGS)"
	@echo "Linker Flags (LDFLAGS): $(LDFLAGS)"


.PHONY: all bin server clean re re-bin re-server run-bin run-server debug

# Prevent deletion of intermediate files
.SECONDARY: $(ALL_OBJECTS)
