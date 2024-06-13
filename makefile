# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra

# Executable name
EXEC = main

# Source files
SRCS = main.cpp board.cpp bitboard.cpp utils.cpp evaluation.cpp move.cpp movegen.cpp transpositionTable.cpp

# Header files
HEADERS = defs.hpp board.hpp bitboard.hpp utils.hpp evaluation.hpp move.hpp movegen.hpp transpositionTable.hpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean
