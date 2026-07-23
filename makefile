CXX = g++
# you can use -march=native if supported
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=x86-64-v2 -flto -DNDEBUG -Isrc -pthread
EXEC = chanakya

BUILD_DIR = build

# Find all .cpp files recursively under src/
SRCS = $(shell find src -name '*.cpp')
OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# -MMD -MP auto-generates per-file header dependencies (.d files)
# so you don't need to hand-maintain a HEADERS list
$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

.PHONY: all clean