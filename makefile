CXX = g++

# Only apply the x86-64-v2 baseline on x86 targets; ARM (Apple Silicon, etc.)
# doesn't use -march=x86-64-v2 and doesn't need it — Clang/GCC already
# target the correct native ISA on ARM without this flag.
ARCH := $(shell uname -m)
ifeq ($(filter $(ARCH),x86_64 amd64),$(ARCH))
    MARCH_FLAG = -march=x86-64-v2
else
    MARCH_FLAG =
endif

CXXFLAGS = -std=c++17 -Wall -Wextra -O3 $(MARCH_FLAG) -flto -DNDEBUG -Isrc -pthread
EXEC = chanakya
BUILD_DIR = build

SRCS = $(shell find src -name '*.cpp')
OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

.PHONY: all clean