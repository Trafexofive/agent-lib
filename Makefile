# Compiler
CXX := clang++
# CXX := g++

# Target Executable Names
TARGET_BIN := agent-bin
TARGET_SERVER := agent-server # *** RENAMED the server executable ***

# Directories
BUILD_DIR := build
INC_DIRS  := inc server/vendor/httplib

# Flags
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2 -g -MMD -MP
CPPFLAGS := $(foreach dir,$(INC_DIRS),-I$(dir))
LDFLAGS := -lcurl -ljsoncpp -pthread

# --- Source Files (Explicitly List ALL with relative paths) ---
MAIN_SRC      := main.cpp
SERVER_SRC    := server/server.cpp # Path to server source

SRC_SOURCES   := $(wildcard src/*.cpp)
EXT_SOURCES   := $(wildcard externals/*.cpp)

COMMON_SOURCES := $(SRC_SOURCES) $(EXT_SOURCES)
ALL_SOURCES := $(MAIN_SRC) $(SERVER_SRC) $(COMMON_SOURCES)

# --- Object Files (Map explicitly using notdir and build path) ---
MAIN_OBJ       := $(BUILD_DIR)/$(notdir $(MAIN_SRC:.cpp=.o))
SERVER_OBJ     := $(BUILD_DIR)/$(notdir $(SERVER_SRC:.cpp=.o))
COMMON_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(COMMON_SOURCES:.cpp=.o)))
ALL_OBJECTS := $(MAIN_OBJ) $(SERVER_OBJ) $(COMMON_OBJECTS)
DEPS := $(ALL_OBJECTS:.o=.d)

# --- Build Rules ---

# Default target builds the SERVER executable
all: $(TARGET_SERVER)

# Rule to build 'bin' executable (uses main.cpp)
$(TARGET_BIN): $(MAIN_OBJ) $(COMMON_OBJECTS) | $(BUILD_DIR)
	@echo "Linking $(TARGET_BIN)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "$(TARGET_BIN) built successfully."

# Rule to build the server executable (now named agent-server)
$(TARGET_SERVER): $(SERVER_OBJ) $(COMMON_OBJECTS) | $(BUILD_DIR)
	@echo "Linking $(TARGET_SERVER)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "$(TARGET_SERVER) built successfully."

# --- Explicit Compilation Rules for EACH file ---

# Ensure build directory exists (Order-only prerequisite)
$(ALL_OBJECTS): | $(BUILD_DIR)
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compile main.cpp
$(BUILD_DIR)/main.o: main.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Compile server/server.cpp
# Target is still build/server.o, Source is server/server.cpp
$(BUILD_DIR)/server.o: server/server.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Compile src files
$(BUILD_DIR)/agent.o: src/agent.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/behavior.o: src/behavior.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/groqClient.o: src/groqClient.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/MiniGemini.o: src/MiniGemini.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/tools.o: src/tools.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/utils.o: src/utils.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Compile externals files
$(BUILD_DIR)/bash.o: externals/bash.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/cal-events.o: externals/cal-events.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/ddg-search.o: externals/ddg-search.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/file.o: externals/file.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/general.o: externals/general.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/search.o: externals/search.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/sway.o: externals/sway.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/write.o: externals/write.cpp
	@echo "Compiling $< -> $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# --- Dependencies ---
-include $(DEPS)

# --- Convenience Targets ---
bin: $(TARGET_BIN)
# Phony target named 'server' depends on the actual executable file 'agent-server'
server: $(TARGET_SERVER)

clean:
	@echo "Cleaning build files..."
	# Remove the specific executable FILES and the build dir
	rm -f $(TARGET_BIN) $(TARGET_SERVER)
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Rebuild targets depend on the phony targets
re: clean all
re-bin: clean bin
re-server: clean server

# Run targets depend on the phony targets
run-bin: bin
	@echo "Running $(TARGET_BIN)..."
	./$(TARGET_BIN)

run-server: server
	@echo "Running $(TARGET_SERVER)..."
	./$(TARGET_SERVER) # Execute the renamed file

.PHONY: all bin server clean re re-bin re-server run-bin run-server

# Prevent deletion of intermediate object files
.SECONDARY: $(ALL_OBJECTS)
