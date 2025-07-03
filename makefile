# Simple Makefile for Order Book System

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

# All source files
SOURCES = Order.cpp OrderModify.cpp Trade.cpp OrderBookLevelInfo.cpp Orderbook.cpp main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executable
TARGET = orderbook

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean rebuild run debug
