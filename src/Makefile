# Chess Engine Makefile
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
DEBUG_FLAGS = -std=c++17 -Wall -Wextra -g -DDEBUG
RELEASE_FLAGS = -std=c++17 -Wall -Wextra -O3 -DNDEBUG

# Directories
SRC_DIR = src
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

# Target executable name
TARGET = chess_engine
DEBUG_TARGET = $(DEBUG_DIR)/$(TARGET)
RELEASE_TARGET = $(RELEASE_DIR)/$(TARGET)

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEBUG_OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(DEBUG_DIR)/%.o)
RELEASE_OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(RELEASE_DIR)/%.o)

# Header dependencies
DEPS = $(OBJECTS:.o=.d)
DEBUG_DEPS = $(DEBUG_OBJECTS:.o=.d)
RELEASE_DEPS = $(RELEASE_OBJECTS:.o=.d)

# Default target
.PHONY: all debug release clean help test

all: release

# Debug build
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(DEBUG_TARGET)

# Release build
release: CXXFLAGS = $(RELEASE_FLAGS)
release: $(RELEASE_TARGET)

# Debug executable
$(DEBUG_TARGET): $(DEBUG_OBJECTS) | $(DEBUG_DIR)
	$(CXX) $(DEBUG_OBJECTS) -o $@
	@echo "Debug build complete: $@"

# Release executable
$(RELEASE_TARGET): $(RELEASE_OBJECTS) | $(RELEASE_DIR)
	$(CXX) $(RELEASE_OBJECTS) -o $@
	@echo "Release build complete: $@"

# Debug object files
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEBUG_DIR)
	$(CXX) $(DEBUG_FLAGS) -MMD -MP -c $< -o $@

# Release object files
$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp | $(RELEASE_DIR)
	$(CXX) $(RELEASE_FLAGS) -MMD -MP -c $< -o $@

# Create build directories
$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Include dependency files
-include $(DEBUG_DEPS)
-include $(RELEASE_DEPS)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Clean and rebuild
rebuild: clean all

# Run the debug version
run-debug: debug
	./$(DEBUG_TARGET)

# Run the release version
run: release
	./$(RELEASE_TARGET)

# Install (optional - copies to /usr/local/bin)
install: release
	sudo cp $(RELEASE_TARGET) /usr/local/bin/$(TARGET)
	@echo "Installed $(TARGET) to /usr/local/bin"

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET) from /usr/local/bin"

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build release version (default)"
	@echo "  debug      - Build debug version with debugging symbols"
	@echo "  release    - Build optimized release version"
	@echo "  clean      - Remove all build artifacts"
	@echo "  rebuild    - Clean and rebuild"
	@echo "  run        - Build and run release version"
	@echo "  run-debug  - Build and run debug version"
	@echo "  install    - Install release binary to system"
	@echo "  uninstall  - Remove installed binary from system"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Build configurations:"
	@echo "  Debug:   $(DEBUG_FLAGS)"
	@echo "  Release: $(RELEASE_FLAGS)"

# Print variables (for debugging the Makefile)
print-%:
	@echo $* = $($*)