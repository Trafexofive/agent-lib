# Compiler
CXX := clang++
# Flags
CXXFLAGS := -Wall -Wextra -pedantic -O2 # Release flags
CPPFLAGS := -Iinclude                   # Include directory
LDFLAGS := -lcurl -ljsoncpp             # Link libraries

# Project Structure
SRC_DIR := src
BUILD_DIR := build
TARGET := agent

# Files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp) main.cpp
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o, $(filter $(SRC_DIR)/%.cpp, $(SOURCES)))
OBJECTS += $(patsubst %.cpp,$(BUILD_DIR)/%.o, $(filter-out $(SRC_DIR)/%.cpp, $(SOURCES)))
HEADERS := $(wildcard include/*.hpp)

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "$(TARGET) built successfully."

# Compile sources from src/ dir
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	@echo "Compiling $< -> $@..."
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Compile sources from root dir (e.g., main.cpp)
$(BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(BUILD_DIR)
	@echo "Compiling $< -> $@..."
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Phony targets
.PHONY: all clean

# Prevent intermediate object file deletion
.SECONDARY: $(OBJECTS)
