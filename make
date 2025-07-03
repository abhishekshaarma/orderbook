# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -DNDEBUG

# Directories
SRC_DIR = .
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = Order.cpp OrderModify.cpp Trade.cpp OrderBookLevelInfo.cpp Orderbook.cpp
MAIN_SRC = main.cpp

# Object files
OBJECTS = $(SOURCES:%.cpp=$(OBJ_DIR)/%.o)
MAIN_OBJ = $(OBJ_DIR)/main.o

# Target executable
TARGET = $(BIN_DIR)/orderbook

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files to create executable
$(TARGET): $(OBJECTS) $(MAIN_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(MAIN_OBJ) -o $@

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Release build
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Clean and rebuild
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

# Install (optional - copies to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/orderbook

# Show help
help:
	@echo "Available targets:"
	@echo "  all       - Build the project (default)"
	@echo "  debug     - Build with debug flags"
	@echo "  release   - Build with release flags"
	@echo "  clean     - Remove build artifacts"
	@echo "  rebuild   - Clean and build"
	@echo "  run       - Build and run the program"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  help      - Show this help message"

# Phony targets
.PHONY: all debug release clean rebuild run install uninstall help

# Dependencies (auto-generated header dependencies)
-include $(OBJECTS:.o=.d)
-include $(MAIN_OBJ:.o=.d)

# Generate dependency files
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -MM $< | sed 's|^[^:]*:|$(OBJ_DIR)/$*.o:|' > $@
