# Compiler
CXX = g++

# Debug flags
CXXFLAGS = -std=c++11 -Wall -Wextra -g -O0

# Executable name
EXEC = chess

# Source files
SRCS = defs.cpp main.cpp board.cpp bitboard.cpp utils.cpp evaluation.cpp move.cpp movegen.cpp transpositionTable.cpp search.cpp perft.cpp uci.cpp polyglot.cpp

# Header files
HEADERS = defs.hpp board.hpp bitboard.hpp utils.hpp evaluation.hpp move.hpp movegen.hpp transpositionTable.hpp search.hpp perft.hpp uci.hpp polyglot.hpp

# Object files
OBJS = $(SRCS:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean